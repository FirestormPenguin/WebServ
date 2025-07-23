#include "HttpRequest.hpp"

Request::Request(const std::string &rawRequest) {
	// Trova la fine degli header
	size_t headerEnd = rawRequest.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		// Fallback per testare se mancano \r
		headerEnd = rawRequest.find("\n\n");
		if (headerEnd != std::string::npos) {
			headerEnd += 2;
		}
	} else {
		headerEnd += 4;
	}
	
	if (headerEnd != std::string::npos) {
		// Parsing degli header usando istringstream (solo per la parte header)
		std::string headersPart = rawRequest.substr(0, headerEnd - 4); // Rimuovi \r\n\r\n
		std::istringstream stream(headersPart);
		parseRequestLine(stream);
		parseHeaders(stream);
		
		// Body: prendi tutto dopo gli header usando substr (mantiene dati binari)
		if (headerEnd < rawRequest.size()) {
			body = rawRequest.substr(headerEnd);
		}
	} else {
		// Nessun body, solo header
		std::istringstream stream(rawRequest);
		parseRequestLine(stream);
		parseHeaders(stream);
	}
}

void Request::parseRequestLine(std::istringstream &stream) {
	std::string line;
	std::getline(stream, line);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::istringstream lineStream(line);
	lineStream >> method >> path >> version;
}

void Request::parseHeaders(std::istringstream &stream) {
	std::string line;
	while (std::getline(stream, line)) {
		if (line == "\r" || line.empty())
			break;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			headers[key] = value;
		}
	}
}

const std::string &Request::getMethod() const {
	return method;
}

const std::string &Request::getPath() const {
	return path;
}

const std::string &Request::getVersion() const {
	return version;
}

const std::map<std::string, std::string> &Request::getHeaders() const {
	return headers;
}

const std::string &Request::getBody() const {
	return body;
}

std::string Request::getHeaderValue(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}
