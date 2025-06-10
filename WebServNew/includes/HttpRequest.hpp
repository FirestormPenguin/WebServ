#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
private:
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;

public:
	HttpRequest(const std::string& rawRequest);

	const std::string& getMethod() const;
	const std::string& getPath() const;
	const std::string& getVersion() const;
	const std::map<std::string, std::string>& getHeaders() const;
};

#endif
