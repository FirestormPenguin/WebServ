#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(int statusCode, const std::string &body) : statusCode(statusCode), body(body)
	{
	initializeStatusMessages();
}

void HttpResponse::initializeStatusMessages()
{
	statusMessages[200] = "OK";
	statusMessages[400] = "Bad Request";
	statusMessages[404] = "Not Found";
	statusMessages[500] = "Internal Server Error";
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << " " 
			 << (statusMessages.count(statusCode) ? statusMessages.at(statusCode) : "Unknown") << "\r\n";
	
	response << "Content-Length: " << body.length() << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Connection: close\r\n";
	
	response << "\r\n";

	response << body;
	
	return response.str();
}
