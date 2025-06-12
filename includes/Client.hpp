#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "HttpRequest.hpp"

class Client {
private:
	int socketFd;
	std::string recvBuffer;
	Request *request;

public:
	Client(int fd);
	~Client();

	int getSocketFd() const;
	void appendToRecvBuffer(const std::string &data);
	bool hasCompleteRequest() const;
	Request *getRequest() const;
	void parseRequest();
};

#endif
