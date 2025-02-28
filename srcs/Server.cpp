#include "../includes/Server.hpp"

Server::Server(int port)
{
	createSocket();
	bindSocket(port);
	listenSocket();
}

Server::~Server()
{
	close(serverSocket);
}

void Server::createSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Error: Cannot create socket" << std::endl;
		exit(1);
	}
}

void Server::bindSocket(int port)
{
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		std::cerr << "Error: Cannot bind socket" << std::endl;
		exit(1);
	}
}

void Server::listenSocket()
{
	if (listen(serverSocket, 10) == -1)
	{
		std::cerr << "Error: Cannot listen on socket" << std::endl;
		exit(1);
	}
	std::cout << "Server is listening on port..." << std::endl;

	struct pollfd serverPollFd;
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;
	clients.push_back(serverPollFd);

}

void Server::handleNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

	if (clientSocket == -1)
	{
		std::cerr << "Error: Cannot accept connection" << std::endl;
		return;
	}

	std::cout << "New client connected! Socket: " << clientSocket << std::endl;

	struct pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clients.push_back(clientPollFd);
}

void Server::handleClient(size_t index)
{
	int clientSocket = clients[index].fd;
	Client client(clientSocket);

	std::string request = client.receiveRequest();
	if (request.empty())
	{
		std::cout << "Client " << clientSocket << " disconnected" << std::endl;
		close(clientSocket);
		clients.erase(clients.begin() + index);
		return;
	}

	std::cout << "Received request from client " << clientSocket << ":\n" << request << std::endl;

	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 13\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Hello, World!";

	client.sendResponse(response);
}

void Server::run()
{
	while (true)
	{
		int pollCount = poll(clients.data(), clients.size(), TIMEOUT);
		if (pollCount == -1) {
			std::cerr << "Error: poll() failed" << std::endl;
			break;
		}

		for (size_t i = 0; i < clients.size(); ++i)
		{
			if (clients[i].revents & POLLIN)
			{
				if (clients[i].fd == serverSocket)
				{
					handleNewConnection();
				}
				else
				{
					handleClient(i);
				}
			}
		}
	}
}