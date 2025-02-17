#include "../includes/WebServ.h"

HttpRequest::HttpRequest(const std::string &rawRequest) {
	parseRequest(rawRequest);
}

void HttpRequest::parseRequest(const std::string &rawRequest) {
	size_t methodEnd = rawRequest.find(' ');
	size_t pathEnd = rawRequest.find(' ', methodEnd + 1);
	method = rawRequest.substr(0, methodEnd);
	path = rawRequest.substr(methodEnd + 1, pathEnd - methodEnd - 1);
	size_t bodyStart = rawRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos)
		body = rawRequest.substr(bodyStart + 4);
}

std::string HttpRequest::getMethod() const { return method; }
std::string HttpRequest::getPath() const { return path; }
std::string HttpRequest::getBody() const { return body; }