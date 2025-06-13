#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "webserv.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

class Client {
private:
	int _fd;
	std::string _recvBuffer;
	bool _requestComplete;
	bool _keepAlive;

	std::string _method;
	std::string _path;
	std::string _httpVersion;

public:
	Client(int fd);
	~Client();

	int getFd() const { return _fd; }
	void appendToBuffer(const std::string& data);
	bool hasCompleteRequest() const;
	void parseRequest();
	std::string prepareResponse(const ServerConfig& config);
	bool isKeepAlive() const;
	void reset();
};

#endif
