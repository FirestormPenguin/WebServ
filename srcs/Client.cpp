#include "Client.hpp"

Client::Client(int fd)
	: _fd(fd), _recvBuffer(""), _requestComplete(false),
	  _keepAlive(false), _method(""), _path(""), _httpVersion("") {}

Client::~Client() {
	close(_fd);
}

int Client::getFd() const {
	return _fd;
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

	std::getline(ss, line);
	std::istringstream lineStream(line);
	lineStream >> _method >> _path >> _httpVersion;

	_keepAlive = false;
	while (std::getline(ss, line) && line != "\r") {
		if (line.find("Connection:") != std::string::npos) {
			if (line.find("keep-alive") != std::string::npos)
				_keepAlive = true;
			else
				_keepAlive = false;
		}
	}

	_requestComplete = true;
}

std::string Client::prepareResponse(const ServerConfig& config) {
	const LocationConfig* loc = config.findLocation(_path);
	std::string filePath;
	if (loc) {
		filePath = loc->getRoot() + "/" + loc->getIndex();
	} else {
		filePath = "www/index.html";
	}

	std::ifstream file(filePath.c_str());
	std::string body;
	if (file) {
		body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	} else {
		body = "<h1>404 Not Found</h1>";
	}

	std::ostringstream oss;
	oss << "HTTP/1.1 " << (file ? "200 OK" : "404 Not Found") << "\r\n";
	oss << "Content-Type: text/html\r\n";
	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "Connection: close\r\n";
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
