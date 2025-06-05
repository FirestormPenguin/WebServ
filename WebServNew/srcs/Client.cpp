#include "Client.hpp"

Client::Client(int fd) : _fd(fd) {}

Client::~Client() {}

int Client::getFd() const {
	return _fd;
}

void Client::appendToRecvBuffer(const std::string& data) {
	_recvBuffer += data;
}

const std::string& Client::getRecvBuffer() const {
	return _recvBuffer;
}

void Client::clearRecvBuffer() {
	_recvBuffer.clear();
}

void Client::setSendBuffer(const std::string& data) {
	_sendBuffer = data;
}

const std::string& Client::getSendBuffer() const {
	return _sendBuffer;
}

void Client::clearSendBuffer() {
	_sendBuffer.clear();
}
