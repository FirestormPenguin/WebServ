#include "HttpRequest.hpp"

Request::Request(const std::string &rawRequest) {
	std::istringstream stream(rawRequest);

	parseRequestLine(stream);
	parseHeaders(stream);
	parseBody(stream);
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

void Request::parseBody(std::istringstream &stream) {
	std::ostringstream bodyStream;
	bodyStream << stream.rdbuf();
	body = bodyStream.str();
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
