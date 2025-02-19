#include "../includes/WebServ.h"
#include "../includes/Client.hpp"

Client::Client(int socket) : clientSocket(socket) {}

Client::Client() {}

Client::~Client() {
	close(clientSocket);
}

std::string Client::receiveRequest() {
	char buffer[1024];
	int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
	
	if (bytesRead <= 0) {
		std::cerr << "Error reading request from client" << std::endl;
		return "";
	}
	
	std::cout << "Bytes read: " << bytesRead << std::endl;
	
	return std::string(buffer, bytesRead);
}


void Client::sendResponse(const std::string &response) {
	ssize_t bytes_sent = send(clientSocket, response.c_str(), response.length(), 0);
	
	if (bytes_sent == -1) {
		std::cerr << "Error sending response" << std::endl;
	} else {
		std::cout << "Response sent (" << bytes_sent << " bytes)" << std::endl;
	}
}
