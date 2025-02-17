#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "WebServ.h"

class HttpRequest {
	public:
		HttpRequest(const std::string &rawRequest);
		std::string getMethod() const;
		std::string getPath() const;
		std::string getBody() const;

	private:
		std::string method;
		std::string path;
		std::string body;
		void parseRequest(const std::string &rawRequest);
};

#endif