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

static std::string getErrorBody(const ServerConfig& config, int code, const std::string& defaultMsg) {
	std::string errorPagePath = config.getErrorPage(code);
	if (!errorPagePath.empty()) {
		std::ifstream errorFile(errorPagePath.c_str());
		if (errorFile)
			return std::string((std::istreambuf_iterator<char>(errorFile)), std::istreambuf_iterator<char>());
	}
	return "<h1>" + defaultMsg + "</h1>";
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

	// Costruisci il percorso reale del file richiesto
	if (path == "/" || path.empty())
		filePath = root + "/" + index;
	else
		filePath = root + path;

	// Limite dimensione body
	if (req.getBody().size() > config.getClientMaxBodySize()) {
		status = "413 Payload Too Large";
		body = getErrorBody(config, 413, "413 Payload Too Large");
	}
	// Metodo non permesso (esempio base, migliora con allow_methods per location)
	else if (method != "GET" && method != "POST" && method != "DELETE") {
		status = "405 Method Not Allowed";
		body = getErrorBody(config, 405, "405 Method Not Allowed");
	}
	else if (method == "GET") {
		struct stat st;
		if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
			// È una directory
			if (loc && loc->getAutoindex()) {
				body = generateAutoindex(filePath, path);
				status = "200 OK";
			} else {
				// Autoindex off: cerca index file o mostra forbidden
				std::string indexPath = filePath + "/" + index;
				std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
				if (indexFile) {
					body.assign((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
					status = "200 OK";
				} else {
					status = "403 Forbidden";
					body = getErrorBody(config, 403, "403 Forbidden");
				}
			}
		} else {
			// Non è una directory: gestisci come file normale
			std::ifstream file(filePath.c_str(), std::ios::binary);
			if (file) {
				body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				status = "200 OK";
			} else {
				status = "404 Not Found";
				body = getErrorBody(config, 404, "404 Not Found");
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
			body = getErrorBody(config, 500, "500 Internal Server Error");
		}
	}
	else if (method == "DELETE") {
		struct stat st;
		if (stat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
			if (remove(filePath.c_str()) == 0) {
				body = "<h1>File deleted!</h1>";
				status = "200 OK";
			} else {
				status = "403 Forbidden";
				body = getErrorBody(config, 403, "403 Forbidden");
			}
		} else {
			status = "404 Not Found";
			body = getErrorBody(config, 404, "404 Not Found");
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
