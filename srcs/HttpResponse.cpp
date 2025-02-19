#include "../includes/WebServ.h"
#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(int statusCode, const std::string &body)
	: statusCode(statusCode), body(body) {}

std::string HttpResponse::intToString(int num) const {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string getStatusMessage(int statusCode) {
	if (statusCode == 200) return "OK";
	if (statusCode == 404) return "Not Found";
	if (statusCode == 500) return "Internal Server Error";
	return "Unknown";
}

std::string HttpResponse::toString() const {
	std::ostringstream oss;
	oss << "HTTP/1.1 " << statusCode << " OK\r\n"
		<< "Content-Length: " << body.length() << "\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Connection: close\r\n\r\n"
		<< body;
	return oss.str();
}
