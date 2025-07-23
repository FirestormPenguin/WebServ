#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

class Request {
private:
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;

	void parseRequestLine(std::istringstream &stream);
	void parseHeaders(std::istringstream &stream);

public:
	Request(const std::string &rawRequest);

	const std::string &getMethod() const;
	const std::string &getPath() const;
	const std::string &getVersion() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;

	std::string getHeaderValue(const std::string &key) const;
};

#endif
