#include "../includes/Client.hpp"

Client::Client(int socket) : clientSocket(socket) {}

Client::~Client()
{
	close(clientSocket);
}

std::string Client::receiveRequest()
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	
	int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0)
	{
		return "";
	}

	return std::string(buffer, bytesRead);
}

void Client::sendResponse(const std::string &response)
{
	send(clientSocket, response.c_str(), response.length(), 0);
}
