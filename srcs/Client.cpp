#include "Client.hpp"
#include <sys/stat.h>  // Per mkdir

// Funzione per determinare il Content-Type basato sull'estensione del file
static std::string getContentType(const std::string& filePath) {
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos) {
		return "application/octet-stream";
	}
	
	std::string ext = filePath.substr(dotPos);
	// Converti in minuscolo
	for (size_t i = 0; i < ext.length(); ++i) {
		if (ext[i] >= 'A' && ext[i] <= 'Z') {
			ext[i] = ext[i] + 32;
		}
	}
	
	// Tipi di file comuni
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".json") return "application/json";
	if (ext == ".txt") return "text/plain";
	
	// Tipi di immagine
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".png") return "image/png";
	if (ext == ".gif") return "image/gif";
	if (ext == ".webp") return "image/webp";
	if (ext == ".bmp") return "image/bmp";
	if (ext == ".ico") return "image/x-icon";
	if (ext == ".svg") return "image/svg+xml";
	
	// Altri tipi comuni
	if (ext == ".pdf") return "application/pdf";
	if (ext == ".zip") return "application/zip";
	if (ext == ".mp4") return "video/mp4";
	if (ext == ".mp3") return "audio/mpeg";
	
	return "application/octet-stream";
}

Client::Client(int fd)
	: _fd(fd), _recvBuffer(""), _requestComplete(false),
	  _keepAlive(false), _sent100Continue(false), _method(""), _path(""), _httpVersion("") {}

Client::~Client() {
	close(_fd);
}

void Client::appendToBuffer(const std::string& data) {
	_recvBuffer += data;
}

bool Client::hasCompleteRequest() const {
	size_t header_end = _recvBuffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return false;

	// CASO SPECIALE: Se ha "Expect: 100-continue" ma non abbiamo ancora inviato la risposta,
	// considera la richiesta "completa" SOLO per permettere l'invio di "100 Continue"
	// MA non per processare la richiesta vera e propria
	if (_recvBuffer.find("Expect: 100-continue") != std::string::npos && !_sent100Continue) {
		return true;
	}

	// CASO SPECIALE: Multipart senza "Expect: 100-continue" (Chrome moderno)
	// Se ha multipart/form-data e header completi, ma non ha inviato "100 Continue",
	// considera "completo" per permettere al main loop di gestire la situazione
	if (_recvBuffer.find("multipart/form-data") != std::string::npos && 
		header_end != std::string::npos && !_sent100Continue) {
		return true;
	}

	// Se abbiamo già inviato "100 Continue", aspetta che arrivi tutto il body
	if (_recvBuffer.find("Expect: 100-continue") != std::string::npos && _sent100Continue) {
		// Forza il controllo del Content-Length per aspettare tutto il body
	}

	// Controlla se usa Transfer-Encoding: chunked (usato dai browser)
	size_t te_pos = _recvBuffer.find("Transfer-Encoding:");
	if (te_pos != std::string::npos && te_pos < header_end) {
		size_t value_start = te_pos + 18;
		while (value_start < _recvBuffer.size() && (_recvBuffer[value_start] == ' ' || _recvBuffer[value_start] == '\t'))
			++value_start;
		size_t value_end = _recvBuffer.find("\r\n", value_start);
		std::string te_str = _recvBuffer.substr(value_start, value_end - value_start);
		
		if (te_str.find("chunked") != std::string::npos) {
			// Per chunked, cerca la fine: "0\r\n\r\n"
			return _recvBuffer.find("0\r\n\r\n") != std::string::npos;
		}
	}

	// Cerca Content-Length (comportamento normale)
	size_t cl_pos = _recvBuffer.find("Content-Length:");
	if (cl_pos != std::string::npos && cl_pos < header_end) {
		size_t value_start = cl_pos + 15;
		while (value_start < _recvBuffer.size() && (_recvBuffer[value_start] == ' ' || _recvBuffer[value_start] == '\t'))
			++value_start;
		size_t value_end = _recvBuffer.find("\r\n", value_start);
		std::string cl_str = _recvBuffer.substr(value_start, value_end - value_start);
		size_t content_length = atoi(cl_str.c_str());
		size_t total_size = header_end + 4 + content_length;
		
		// Per i multipart grandi, se abbiamo ricevuto almeno il 98% considera completo
		// Questo gestisce problemi di rete dove mancano pochi byte finali
		bool isComplete = _recvBuffer.size() >= total_size;
		if (!isComplete && _recvBuffer.find("multipart/form-data") != std::string::npos) {
			if (_recvBuffer.size() >= (total_size * 98) / 100) {
				return true;
			}
		}
		
		return isComplete;
	}
	// Nessun Content-Length: solo header
	return true;
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

		char* argv[] = {const_cast<char*>(cgiBin.c_str()), const_cast<char*>(scriptPath.c_str()), NULL};

		std::string scriptFilename = scriptPath;
		std::string requestMethod = req.getMethod();
		std::ostringstream oss;
		oss << req.getBody().size();
		std::string contentLength = oss.str();
		std::string contentType = "application/x-www-form-urlencoded";
		std::string queryString = "";
		std::string redirectEnv = "REDIRECT_STATUS=1";

		std::string scriptEnv = "SCRIPT_FILENAME=" + scriptFilename;
		std::string methodEnv = "REQUEST_METHOD=" + requestMethod;
		std::string lengthEnv = "CONTENT_LENGTH=" + contentLength;
		std::string typeEnv   = "CONTENT_TYPE=" + contentType;
		std::string queryEnv  = "QUERY_STRING=" + queryString;

		char* envp[] = {
			const_cast<char*>(scriptEnv.c_str()),
			const_cast<char*>(methodEnv.c_str()),
			const_cast<char*>(lengthEnv.c_str()),
			const_cast<char*>(typeEnv.c_str()),
			const_cast<char*>(queryEnv.c_str()),
			const_cast<char*>(redirectEnv.c_str()),
			NULL
		};

		execve(cgiBin.c_str(), argv, envp);
		const char* errMsg = "Status: 500 Internal Server Error\r\n\r\nCGI execution failed.\n";
		write(STDOUT_FILENO, errMsg, strlen(errMsg));
		while (1) {}
	} else {
		// Padre: scrivi body su stdin del CGI, leggi output con timeout
		close(inPipe[0]);
		close(outPipe[1]);
		write(inPipe[1], req.getBody().c_str(), req.getBody().size());
		close(inPipe[1]);

		std::string cgiOutput;
		char buf[4096];
		ssize_t n;
		int status = 0;
		int timeout_sec = 5; // Timeout di 5 secondi

		fd_set readfds;
		struct timeval tv;
		FD_ZERO(&readfds);
		FD_SET(outPipe[0], &readfds);
		tv.tv_sec = timeout_sec;
		tv.tv_usec = 0;

		int sel = select(outPipe[0] + 1, &readfds, NULL, NULL, &tv);
		if (sel > 0 && FD_ISSET(outPipe[0], &readfds)) {
			while ((n = read(outPipe[0], buf, sizeof(buf))) > 0) {
				cgiOutput.append(buf, n);
			}
			close(outPipe[0]);
			waitpid(pid, &status, 0);
			return cgiOutput;
		} else {
			// Timeout scaduto: termina il CGI e restituisci errore 500
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			close(outPipe[0]);
			return "Status: 500 Internal Server Error\r\n\r\nCGI timeout.\n";
		}
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
	std::string contentType = "text/html"; // Default, sarà cambiato se necessario
	std::string filePath;

	// CASO SPECIALE: Se la richiesta ha "Expect: 100-continue" e non abbiamo ancora tutto il body,
	// NON processare ancora la richiesta
	if (_recvBuffer.find("Expect: 100-continue") != std::string::npos) {
		size_t header_end = _recvBuffer.find("\r\n\r\n");
		if (header_end != std::string::npos) {
			// Controlla se abbiamo tutto il body basandoci su Content-Length
			size_t cl_pos = _recvBuffer.find("Content-Length:");
			if (cl_pos != std::string::npos && cl_pos < header_end) {
				size_t value_start = cl_pos + 15;
				while (value_start < _recvBuffer.size() && (_recvBuffer[value_start] == ' ' || _recvBuffer[value_start] == '\t'))
					++value_start;
				size_t value_end = _recvBuffer.find("\r\n", value_start);
				std::string cl_str = _recvBuffer.substr(value_start, value_end - value_start);
				size_t content_length = atoi(cl_str.c_str());
				size_t total_size = header_end + 4 + content_length;
				
				if (_recvBuffer.size() < total_size) {
					return "";
				}
			} else {
				// Se non c'è Content-Length ma c'è "Expect: 100-continue", aspetta
				if (req.getBody().empty()) {
					return "";
				}
			}
		}
	}

	const LocationConfig* loc = config.findLocation(path);
	std::string root = loc ? loc->getRoot() : "www";
	std::string index = "";
	if (loc && !loc->getIndex().empty()) {
		index = loc->getIndex();
	} else {
		index = "index.html"; // default fallback
	}

	// Costruisci il path reale del file o directory richiesto
	if (path == "/" || path.empty())
		filePath = root + "/";
	else
		filePath = root + path;

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
		struct stat st;
		if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
			// Directory: cerca index file
			std::string dirPath = filePath;
			if (dirPath[dirPath.size()-1] != '/')
				dirPath += "/";
			
			// Se c'è una regola index specifica nella location, prova a cercarla
			bool hasIndexRule = (loc && !loc->getIndex().empty());
			
			if (hasIndexRule) {
				// C'è una regola index esplicita, prova a cercare il file
				std::string indexPath = dirPath + loc->getIndex();
				std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
				if (indexFile.is_open()) {
					body.assign((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
					status = "200 OK";
				} else {
					// Index file specificato ma non trovato
					if (loc && loc->getAutoindex()) {
						body = generateAutoindex(dirPath, path);
						status = "200 OK";
					} else {
						status = "403 Forbidden";
						body = getErrorBody(config, 403, "403 Forbidden", root);
					}
				}
			} else {
				// Nessuna regola index esplicita, usa autoindex se abilitato
				if (loc && loc->getAutoindex()) {
					body = generateAutoindex(dirPath, path);
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
				} else {
					// Separa header CGI dal body
					size_t headerEnd = cgiOutput.find("\r\n\r\n");
					if (headerEnd != std::string::npos) {
						body = cgiOutput.substr(headerEnd + 4);
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
					contentType = getContentType(filePath); // Imposta il Content-Type corretto
				} else {
					status = "404 Not Found";
					body = getErrorBody(config, 404, "404 Not Found", root);
				}
			}
		}
	}
	else if (method == "POST") {
		std::string contentTypeHeader = req.getHeaderValue("Content-Type");
		
		// Controlla se è un upload di file multipart/form-data
		if (contentTypeHeader.find("multipart/form-data") != std::string::npos) {
			// Estrai il boundary dal Content-Type
			std::string boundary;
			size_t boundaryPos = contentTypeHeader.find("boundary=");
			if (boundaryPos != std::string::npos) {
				boundary = contentTypeHeader.substr(boundaryPos + 9);
				// Rimuovi eventuali spazi o caratteri extra
				size_t endPos = boundary.find_first_of(" ;\r\n");
				if (endPos != std::string::npos) {
					boundary = boundary.substr(0, endPos);
				}
			}
			
			// Parse multipart data
			std::string bodyData = req.getBody();
			std::string fullBoundary = "--" + boundary;
			
			size_t pos = 0;
			std::string uploadedFile = "";
			std::string filename = "";
			std::string contentType = "";
			
			while ((pos = bodyData.find(fullBoundary, pos)) != std::string::npos) {
				pos += fullBoundary.length();
				
				// Trova la fine di questa parte
				size_t nextBoundary = bodyData.find(fullBoundary, pos);
				if (nextBoundary == std::string::npos) break;
				
				std::string part = bodyData.substr(pos, nextBoundary - pos);
				
				// Trova la separazione header/body di questa parte
				size_t headerEnd = part.find("\r\n\r\n");
				if (headerEnd == std::string::npos) continue;
				
				std::string partHeaders = part.substr(0, headerEnd);
				std::string partBody = part.substr(headerEnd + 4);
				
				// Se questa parte ha un filename, è il file
				if (partHeaders.find("filename=") != std::string::npos) {
					// Estrai il filename
					size_t filenameStart = partHeaders.find("filename=\"");
					if (filenameStart != std::string::npos) {
						filenameStart += 10; // Lunghezza di 'filename="'
						size_t filenameEnd = partHeaders.find("\"", filenameStart);
						if (filenameEnd != std::string::npos) {
							filename = partHeaders.substr(filenameStart, filenameEnd - filenameStart);
						}
					}
					
					// Estrai il Content-Type
					size_t ctStart = partHeaders.find("Content-Type:");
					if (ctStart != std::string::npos) {
						ctStart += 13; // Lunghezza di 'Content-Type:'
						size_t ctEnd = partHeaders.find("\r\n", ctStart);
						if (ctEnd != std::string::npos) {
							contentType = partHeaders.substr(ctStart, ctEnd - ctStart);
							// Rimuovi spazi iniziali
							while (!contentType.empty() && contentType[0] == ' ') {
								contentType = contentType.substr(1);
							}
						}
					}
					
					// Rimuovi \r\n finale dal body se presente
					if (partBody.length() >= 2 && partBody.substr(partBody.length() - 2) == "\r\n") {
						partBody = partBody.substr(0, partBody.length() - 2);
					}
					
					uploadedFile = partBody;
					break;
				}
			}
			
			if (!uploadedFile.empty() && !filename.empty()) {
				// Crea directory images se non esiste
				std::string imagesDir = root + "/images";
				mkdir(imagesDir.c_str(), 0755);
				
				// Salva il file con il nome originale
				std::string imagePath = imagesDir + "/" + filename;
				std::ofstream imageOut(imagePath.c_str(), std::ios::binary);
				if (imageOut) {
					imageOut.write(uploadedFile.c_str(), uploadedFile.size());
					imageOut.close();
					
					// Crea una pagina HTML che mostra l'immagine uploadata
					std::ostringstream response;
					response << "<html><head><title>Upload Success</title></head><body>";
					response << "<h1>Image uploaded successfully!</h1>";
					response << "<p>File: " << filename << "</p>";
					response << "<p>Type: " << contentType << "</p>";
					response << "<p>Size: " << uploadedFile.size() << " bytes</p>";
					response << "<img src=\"/images/" << filename << "\" style=\"max-width: 500px; height: auto;\" alt=\"" << filename << "\">";
					response << "<br><br><a href=\"/upload.html\">Upload another image</a>";
					response << "</body></html>";
					
					body = response.str();
					status = "201 Created";
				} else {
					status = "500 Internal Server Error";
					body = getErrorBody(config, 500, "500 Failed to save image", root);
				}
			} else {
				status = "400 Bad Request";
				body = getErrorBody(config, 400, "400 No valid file found in upload", root);
			}
		} else {
			// Upload normale (come prima)
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
	_sent100Continue = false;
	_method.clear();
	_path.clear();
	_httpVersion.clear();
}
