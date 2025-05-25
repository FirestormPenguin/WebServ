#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string &rawRequest)
{
	parseRequest(rawRequest);
}

void HttpRequest::parseRequest(const std::string &rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string line;

	if (std::getline(requestStream, line))
	{
		std::istringstream lineStream(line);
		lineStream >> method >> path >> version;
	}

	while (std::getline(requestStream, line) && line != "\r")
	{
		size_t delimiter = line.find(": ");
		if (delimiter != std::string::npos) {
			std::string key = line.substr(0, delimiter);
			std::string value = line.substr(delimiter + 2);
			if (!value.empty() && value[value.size() - 1] == '\r')
			{
				value.erase(value.size() - 1);
			}
			headers[key] = value;
		}
	}

	if (headers.find("Content-Length") != headers.end())
	{
		int contentLength = atoi(headers["Content-Length"].c_str());
		char *buffer = new char[contentLength + 1];
		requestStream.read(buffer, contentLength);
		buffer[contentLength] = '\0';
		body = std::string(buffer);
		delete[] buffer;
	}
}

std::string HttpRequest::getMethod() const { return method; }

std::string HttpRequest::getPath() const { return path; }

std::string HttpRequest::getVersion() const { return version; }

std::string HttpRequest::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end()) return it->second;
	return "";
}

std::string HttpRequest::getBody() const { return body; }
