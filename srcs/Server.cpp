#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <cstdlib>

Server::Server(const std::string &configFile)
{
	Config config(configFile);
	port = std::atoi(config.get("port").c_str());
	serverName = config.get("server_name");
	rootDir = config.get("root");

	std::cout << "Server Name: " << serverName << std::endl;
	std::cout << "Set listening on port: " << port << std::endl;
	std::cout << "Root directory: " << rootDir << std::endl;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Error creating socket" << std::endl;
		exit(1);
	}

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error binding socket" << std::endl;
		exit(1);
	}

	if (listen(serverSocket, SOMAXCONN) == -1)
	{
		std::cerr << "Error listening on socket" << std::endl;
		exit(1);
	}

	std::cout << "Server is listening on port " << port << std::endl;

	pollfd serverPollFd;
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;
	pollFds.push_back(serverPollFd);

	
}

Server::~Server()
{
	close(serverSocket);
}

void Server::run()
{
	while (true)
	{
		if (poll(&pollFds[0], pollFds.size(), -1) == -1)
		{
			std::cerr << "Error in poll" << std::endl;
			exit(1);
		}

		for (size_t i = 0; i < pollFds.size(); ++i) 
		{
			if (pollFds[i].revents & POLLIN) 
			{
				if (pollFds[i].fd == serverSocket) 
				{
					acceptConnection();
				} 
				else 
				{
					handleClient(pollFds[i].fd);
				}
			}
			else if (pollFds[i].revents & (POLLHUP | POLLERR)) 
			{
				std::cout << "Client " << pollFds[i].fd << " disconnected or error occurred." << std::endl;
				close(pollFds[i].fd);
				pollFds.erase(pollFds.begin() + i);
				--i;
			}
		}
	}
}

void Server::acceptConnection()
{
	int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting connection" << std::endl;
		return;
	}

	struct pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	pollFds.push_back(clientPollFd);

	clients.insert(std::make_pair(clientSocket, Client(clientSocket)));

	std::cout << "New client connected! Socket: " << clientSocket << std::endl;
}

void Server::handleClient(int fd)
{
	std::map<int, Client>::iterator it = clients.find(fd);
	std::string request;
	if (it != clients.end())
	{
		request = it->second.receiveRequest();
	}
	
	if (request.empty())
	{
		std::cerr << "Client " << fd << " disconnected or sent an invalid request." << std::endl;
		close(fd);
		return;
	}

	HttpRequest httpRequest(request);
	HttpResponse httpResponse(200, "Hello, World!");

	std::map<int, Client>::iterator it2 = clients.find(fd);
	if (it2 != clients.end())
	{
		it2->second.sendResponse(httpResponse.toString());
	}

	std::cout << "Response sent to client " << fd << std::endl;

	close(fd);
}


void Server::closeClient(int fd)
{
	std::cout << "Closing connection: " << fd << std::endl;
	close(fd);
	clients.erase(fd);

	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
		{
			pollFds.erase(pollFds.begin() + i);
			break;
		}
	}
}

void Server::shutdown()
{
	std::cout << "Closing all connections..." << std::endl;
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		close(pollFds[i].fd);
	}
	pollFds.clear();
	close(serverSocket);
}
