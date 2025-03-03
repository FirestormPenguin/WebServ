#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Server.hpp"
#include <string>
#include <map>

class HttpRequest
{
	public:
		HttpRequest(const std::string &rawRequest);
		
		std::string getMethod() const;
		std::string getPath() const;
		std::string getVersion() const;
		std::string getHeader(const std::string &key) const;
		std::string getBody() const;

	private:
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;

		void parseRequest(const std::string &rawRequest);
};

#endif
