#include "../includes/WebServ.h"

HttpResponse::HttpResponse(int statusCode, const std::string &body)
	: statusCode(statusCode), body(body) {}

std::string HttpResponse::toString() const {
	return "HTTP/1.1 " + std::to_string(statusCode) + " OK\r\n"
		   "Content-Length: " + std::to_string(body.length()) + "\r\n"
		   "Content-Type: text/plain\r\n\r\n" + body;
}