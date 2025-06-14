#include "Client.hpp"

Client::Client(int fd)
	: _fd(fd), _recvBuffer(""), _requestComplete(false),
	  _keepAlive(false), _method(""), _path(""), _httpVersion("") {}

Client::~Client() {
	close(_fd);
}

void Client::appendToBuffer(const std::string& data) {
	_recvBuffer += data;
}

bool Client::hasCompleteRequest() const {
	return _recvBuffer.find("\r\n\r\n") != std::string::npos;
}

void Client::parseRequest() {
	std::istringstream ss(_recvBuffer);
	std::string line;
	_keepAlive = false;

	while (std::getline(ss, line) && line != "\r") {
		if (line.find("Connection:") != std::string::npos) {
			if (line.find("keep-alive") != std::string::npos)
				_keepAlive = true;
		}
	}
	_requestComplete = true;
}

static std::string executeCgi(const std::string& scriptPath, const std::string& cgiBin, const Request& req, const std::string& root) {
	(void)root;
	int inPipe[2], outPipe[2];
	pipe(inPipe);
	pipe(outPipe);

	pid_t pid = fork();
	if (pid == 0) {
		// Figlio: setup pipe e execve
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		close(inPipe[1]);
		close(outPipe[0]);

		// Prepara argv
		char* argv[] = {const_cast<char*>(cgiBin.c_str()), const_cast<char*>(scriptPath.c_str()), NULL};

		// Prepara envp (solo le variabili base, puoi aggiungerne altre)
		std::string scriptFilename = scriptPath;
		std::string requestMethod = req.getMethod();
		std::ostringstream oss;
		oss << req.getBody().size();
		std::string contentLength = oss.str();
		std::string contentType = "application/x-www-form-urlencoded";
		std::string queryString = ""; // puoi estrarre dalla URL se serve

		char* envp[] = {
			const_cast<char*>(("SCRIPT_FILENAME=" + scriptFilename).c_str()),
			const_cast<char*>(("REQUEST_METHOD=" + requestMethod).c_str()),
			const_cast<char*>(("CONTENT_LENGTH=" + contentLength).c_str()),
			const_cast<char*>(("CONTENT_TYPE=" + contentType).c_str()),
			const_cast<char*>(("QUERY_STRING=" + queryString).c_str()),
			NULL
		};

		execve(cgiBin.c_str(), argv, envp);
		exit(1); // Se execve fallisce
	} else {
		// Padre: scrivi body su stdin del CGI, leggi output
		close(inPipe[0]);
		close(outPipe[1]);
		write(inPipe[1], req.getBody().c_str(), req.getBody().size());
		close(inPipe[1]);

		std::string cgiOutput;
		char buf[4096];
		ssize_t n;
		while ((n = read(outPipe[0], buf, sizeof(buf))) > 0) {
			cgiOutput.append(buf, n);
		}
		close(outPipe[0]);
		waitpid(pid, NULL, 0);

		return cgiOutput;
	}
}

static std::string generateAutoindex(const std::string& dirPath, const std::string& urlPath) {
	DIR* dir = opendir(dirPath.c_str());
	if (!dir)
		return "<h1>403 Forbidden</h1>";

	std::ostringstream body;
	body << "<html><body><h1>Index of " << urlPath << "</h1><ul>";
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == ".") continue;
		std::string href = urlPath;
		if (href.empty() || href[href.size()-1] != '/')
			href += "/";
		href += name;
		body << "<li><a href=\"" << href << "\">" << name << "</a></li>";
	}
	closedir(dir);
	body << "</ul></body></html>";
	return body.str();
}

static std::string getErrorBody(const ServerConfig& config, int code, const std::string& defaultMsg, const std::string& root) {
	std::string errorPagePath = config.getErrorPage(code);
	if (!errorPagePath.empty()) {
		std::string fullPath = root + errorPagePath; // aggiungi il root davanti!
		std::ifstream errorFile(fullPath.c_str());
		if (errorFile)
			return std::string((std::istreambuf_iterator<char>(errorFile)), std::istreambuf_iterator<char>());
	}
	return "<h1>" + defaultMsg + "</h1>";
}

std::string Client::prepareResponse(const ServerConfig& config) {
	Request req(_recvBuffer);
	std::string method = req.getMethod();
	std::string path = req.getPath();
	std::string body;
	std::string status = "200 OK";
	std::string contentType = "text/html";
	std::string filePath;

	const LocationConfig* loc = config.findLocation(path);
	std::string root = loc ? loc->getRoot() : "www";
	std::string index = loc ? loc->getIndex() : "index.html";

	// --- REDIRECT ---
	if (loc && loc->getRedirectCode() && !loc->getRedirectUrl().empty()) {
		std::ostringstream oss;
		oss << "HTTP/1.1 " << loc->getRedirectCode() << " Redirect\r\n";
		oss << "Location: " << loc->getRedirectUrl() << "\r\n";
		oss << "Content-Length: 0\r\n";
		oss << "Connection: close\r\n\r\n";
		return oss.str();
	}

	// --- ALLOW_METHODS ---
	if (loc && !loc->isMethodAllowed(method)) {
		status = "405 Method Not Allowed";
		body = getErrorBody(config, 405, "405 Method Not Allowed", root);
	}
	// --- MAX BODY SIZE ---
	else if (req.getBody().size() > config.getClientMaxBodySize()) {
		status = "413 Payload Too Large";
		body = getErrorBody(config, 413, "413 Payload Too Large", root);
	}
	// --- METODI ---
	else if (method == "GET") {
		// Costruisci il percorso reale del file richiesto
		if (path == "/" || path.empty())
			filePath = root + "/" + index;
		else
			filePath = root + path;

		struct stat st;
		if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
			if (loc && loc->getAutoindex()) {
				body = generateAutoindex(filePath, path);
				status = "200 OK";
			} else {
				std::string indexPath = filePath + "/" + index;
				std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
				if (indexFile) {
					body.assign((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
					status = "200 OK";
				} else {
					status = "403 Forbidden";
					body = getErrorBody(config, 403, "403 Forbidden", root);
				}
			}
		} else {
			// --- CGI CHECK ---
			bool isCgi = false;
			std::string ext;
			if (loc) {
				size_t dot = filePath.find_last_of('.');
				if (dot != std::string::npos)
					ext = filePath.substr(dot);
				if (!ext.empty() && loc->getCgiMap().count(ext))
					isCgi = true;
			}
			if (isCgi) {
				std::string cgiBin = loc->getCgiMap().find(ext)->second;
				std::string cgiOutput = executeCgi(filePath, cgiBin, req, root);
				if (cgiOutput.empty()) {
					status = "500 Internal Server Error";
					body = getErrorBody(config, 500, "500 Internal Server Error", root);
				}else {
					// Separa header CGI dal body
					size_t headerEnd = cgiOutput.find("\r\n\r\n");
					if (headerEnd != std::string::npos) {
						body = cgiOutput.substr(headerEnd + 4);
						// Puoi anche parsare Content-Type, ecc. dagli header CGI se vuoi
					} else {
						body = cgiOutput;
					}
					status = "200 OK";
				}
			} else {
				std::ifstream file(filePath.c_str(), std::ios::binary);
				if (file) {
					body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					status = "200 OK";
				} else {
					status = "404 Not Found";
					body = getErrorBody(config, 404, "404 Not Found", root);
				}
			}
		}
	}
	else if (method == "POST") {
		std::string uploadPath = root + "/upload.data";
		std::ofstream out(uploadPath.c_str(), std::ios::binary);
		if (out) {
			out << req.getBody();
			out.close();
			body = "<h1>File uploaded!</h1>";
			status = "201 Created";
		} else {
			status = "500 Internal Server Error";
			body = getErrorBody(config, 500, "500 Internal Server Error", root);
		}
	}
	else if (method == "DELETE") {
		if (path == "/" || path.empty())
			filePath = root + "/" + index;
		else
			filePath = root + path;

		struct stat st;
		if (stat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
			if (remove(filePath.c_str()) == 0) {
				body = "<h1>File deleted!</h1>";
				status = "200 OK";
			} else {
				status = "403 Forbidden";
				body = getErrorBody(config, 403, "403 Forbidden", root);
			}
		} else {
			status = "404 Not Found";
			body = getErrorBody(config, 404, "404 Not Found", root);
		}
	}

	std::ostringstream oss;
	oss << "HTTP/1.1 " << status << "\r\n";
	oss << "Content-Type: " << contentType << "\r\n";
	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "Connection: " << (_keepAlive ? "keep-alive" : "close") << "\r\n";
	oss << "\r\n";
	oss << body;
	return oss.str();
}

bool Client::isKeepAlive() const {
	return _keepAlive;
}

void Client::reset() {
	_recvBuffer.clear();
	_requestComplete = false;
	_method.clear();
	_path.clear();
	_httpVersion.clear();
}
