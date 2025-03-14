#include "../includes/Client.hpp"

Client::Client(int socket) : clientSocket(socket) {}

Client::~Client()
{
	close(clientSocket);
}

// std::string Client::receiveRequest()
// {
// 	char buffer[1024];
// 	memset(buffer, 0, sizeof(buffer));
// 	std::string request;
// 	size_t bytesRead;

// 	// while (bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0))
// 	while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) 
// 	{
// 		buffer[bytesRead] = '\0';
// 		request += buffer;
// 		if (request.find("\r\n\r\n") != std::string::npos) 
// 		{
// 			break;
// 		}
// 	}
// 	if (bytesRead <= 0)
// 	{
// 		std::cerr << "Error reading from client socket " << clientSocket << std::endl;
// 		return "";
// 	}

// 	return request;
// }

std::string Client::receiveRequest()
{
	char buffer[1024];
	std::string request;
	ssize_t bytesRead;

	while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) 
	{
		request.append(buffer, bytesRead); // Evita buffer overflow

		// Controlla se gli header sono stati completamente ricevuti
		size_t headerEnd = request.find("\r\n\r\n");
		if (headerEnd != std::string::npos) 
		{
			// Verifica se c'è un body (solo per richieste POST)
			size_t contentLengthPos = request.find("Content-Length:");
			if (contentLengthPos != std::string::npos) 
			{
				size_t contentStart = headerEnd + 4; // Dopo "\r\n\r\n"
				size_t contentLength = atoi(request.substr(contentLengthPos + 15).c_str());

				// Leggi il body se non è stato ancora completamente ricevuto
				while (request.size() < contentStart + contentLength) 
				{
					bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
					if (bytesRead <= 0) break; // Errore o connessione chiusa
					request.append(buffer, bytesRead);
				}
			}
			break; // Abbiamo ricevuto tutto
		}
	}

	if (bytesRead == -1)
	{
		std::cerr << "Error reading from client socket " << clientSocket << ": " << strerror(errno) << std::endl;
		return "";
	}

	return request;
}


int Client::sendResponse(const std::string &response)
{
	int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == -1)
	{
		std::cerr << "Error: send() failed for client " << clientSocket << std::endl;
	}
	return bytesSent;
}
