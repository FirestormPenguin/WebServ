#include "HttpRequest.hpp"
#include <sstream>
#include <iostream>

HttpRequest::HttpRequest(const std::string& rawRequest) {
	std::istringstream stream(rawRequest);
	std::string line;

	// Legge la prima riga: metodo, path, versione
	if (std::getline(stream, line)) {
		std::istringstream lineStream(line);
		lineStream >> method >> path >> version;
	}

	// Legge gli header
	while (std::getline(stream, line) && line != "\r") {
		size_t colon = line.find(":");
		if (colon != std::string::npos) {
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			// Pulisce eventuali spazi e \r
			while (!value.empty() && (value[0] == ' ' || value[0] == '\r'))
				value.erase(0, 1);
			while (!value.empty() && (value[value.size() - 1] == '\r'))
				value.erase(value.size() - 1);
			headers[key] = value;
		}
	}
}

const std::string& HttpRequest::getMethod() const {
	return method;
}

const std::string& HttpRequest::getPath() const {
	return path;
}

const std::string& HttpRequest::getVersion() const {
	return version;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
	return headers;
}
