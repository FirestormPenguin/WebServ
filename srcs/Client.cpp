#include "../includes/WebServ.h"

Client::Client(int socket) : clientSocket(socket) {}

Client::Client() {}

Client::~Client() {
	close(clientSocket);
}

std::string Client::receiveRequest() {
	char buffer[1024];
	int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesRead <= 0) return "";
	return std::string(buffer, bytesRead);
}

void Client::sendResponse(const std::string &response) {
	send(clientSocket, response.c_str(), response.length(), 0);
}