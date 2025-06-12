#include "Client.hpp"
#include <sstream>

Client::Client(int fd) : socketFd(fd), recvBuffer(""), request(NULL) {}

Client::~Client() {
	if (request)
		delete request;
}

int Client::getSocketFd() const {
	return socketFd;
}

void Client::appendToRecvBuffer(const std::string &data) {
	recvBuffer += data;
}

bool Client::hasCompleteRequest() const {
	// Verifica semplice: termina con doppio CRLF
	return recvBuffer.find("\r\n\r\n") != std::string::npos;
}

void Client::parseRequest() {
	if (request)
		delete request;
	request = new Request(recvBuffer);
}

Request *Client::getRequest() const {
	return request;
}
