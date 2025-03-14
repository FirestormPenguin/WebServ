#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <cstdlib>
#include <fcntl.h> 

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

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error binding socket" << std::endl;
		close(serverSocket);
		exit(1);
	}

	if (listen(serverSocket, 10) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		close(serverSocket);
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
			continue;	
		}

		std::vector<size_t> toRemove;
		usleep(1000);
		for (size_t i = 0; i < pollFds.size(); ++i)
		{
			std::cout << "Socket " << pollFds[i].fd << " revents: " << pollFds[i].revents << std::endl;

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
				clients.erase(pollFds[i].fd);
				toRemove.push_back(i);
			}
		}
		for (size_t i = 0; i < toRemove.size(); ++i)
		{
			pollFds.erase(pollFds.begin() + toRemove[i] - i);
		}
	}
}

void Server::acceptConnection()
{
	int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket < 0)
	{
		std::cerr << "Error accepting connection" << std::endl;
		return;
	}

	// struct pollfd clientPollFd;
	// clientPollFd.fd = clientSocket;
	// clientPollFd.events = POLLIN;
	// clientPollFd.revents = 0;
	// pollFds.push_back(clientPollFd);

	// clients.insert(std::make_pair(clientSocket, Client(clientSocket)));

	// std::cout << "Client socket " << clientSocket << " added to pollFds." << std::endl;

	struct sockaddr_in addr;
socklen_t addrLen = sizeof(addr);
if (getpeername(clientSocket, (struct sockaddr*)&addr, &addrLen) == 0)
{
    std::cout << "Client connected from: " << inet_ntoa(addr.sin_addr) 
              << ":" << ntohs(addr.sin_port) << std::endl;
}
else
{
    std::cerr << "Error getting client address: " << strerror(errno) << std::endl;
}

// 🔍 Controlliamo lo stato del socket con getsockopt()
int error = 0;
socklen_t len = sizeof(error);
if (getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
{
    std::cerr << "Error getting socket status: " << strerror(errno) << std::endl;
}
else if (error != 0)
{
    std::cerr << "Socket " << clientSocket << " error: " << strerror(error) << std::endl;
}

// 🔍 Controlliamo i flag del socket
int flags = fcntl(clientSocket, F_GETFL, 0);
if (flags == -1)
{
    std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
}
else
{
    std::cout << "Socket " << clientSocket << " flags: " << flags << std::endl;
}
}

void Server::handleClient(int fd)
{
	std::cout << "Handling client: " << fd << std::endl;
	std::map<int, Client>::iterator it = clients.find(fd);
	
	if (it == clients.end()) return;
	std::string request = it->second.receiveRequest();
	
	if (request.empty())
	{
		std::cerr << "Client " << fd << " disconnected or sent an invalid request." << std::endl;
		close(fd);
		clients.erase(fd);
		return;
	}

	HttpRequest httpRequest(request);
	std::string method = httpRequest.getMethod();
	std::string path = httpRequest.getPath();
	std::string responseBody;

	if (method == "GET")
	{
		responseBody = "<html><body><h1>GET request received</h1></body></html>";
	}
	else if (method == "POST")
	{
		std::string postData = httpRequest.getBody();
		std::cout << "Received POST data: " << postData << std::endl;
		std::ofstream outFile("uploads/data.txt", std::ios::app);
		if (outFile.is_open())
		{
			outFile << postData << "\n";
			outFile.close();
		}
		responseBody = "POST data received: " + postData;
	}
	else
	{
		responseBody = "405 Method Not Allowed";
	}

	int statusCode = (method == "POST" || method == "GET") ? 200 : 405;
	HttpResponse httpRes(statusCode, responseBody);

	it->second.sendResponse(httpRes.toString());

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
