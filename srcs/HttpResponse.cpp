#include "../includes/WebServ.h"

HttpResponse::HttpResponse(int statusCode, const std::string &body)
	: statusCode(statusCode), body(body) {}

std::string HttpResponse::intToString(int num) const {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string HttpResponse::toString() const {
	return "HTTP/1.1 " + intToString(statusCode) + " OK\r\n"
		   "Content-Length: " + intToString(body.length()) + "\r\n"
		   "Content-Type: text/plain\r\n\r\n" + body;
}
