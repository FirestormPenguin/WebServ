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

std::string HttpResponse::getContentType(const std::string& path) const {
	std::map<std::string, std::string> contentTypes;
	contentTypes[".html"] = "text/html";
	contentTypes[".css"] = "text/css";
	contentTypes[".js"] = "application/javascript";
	contentTypes[".png"] = "image/png";
	contentTypes[".jpg"] = "image/jpeg";
	contentTypes[".jpeg"] = "image/jpeg";
	contentTypes[".gif"] = "image/gif";

	size_t pos = path.find_last_of('.');
	if (pos != std::string::npos) {
		std::string ext = path.substr(pos);
		if (contentTypes.find(ext) != contentTypes.end()) {
			return contentTypes[ext];
		}
	}
	return "application/octet-stream";
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << " " 
			 << (statusMessages.count(statusCode) ? statusMessages.at(statusCode) : "Unknown") << "\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Content-Type: " << getContentType("index.html") << "\r\n"
			 << "Connection: close\r\n\r\n"
			 << body;
	return response.str();
}
