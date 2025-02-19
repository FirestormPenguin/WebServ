#include "../includes/WebServ.h"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

Server::Server(int port) : running(true){
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		std::cerr << "Error creating socket" << std::endl;
		exit(1);
	}

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		std::cerr << "Error binding socket" << std::endl;
		exit(1);
	}
	if (listen(serverSocket, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket" << std::endl;
		exit(1);
	}
	pollfd serverPollFd;
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;
	pollFds.push_back(serverPollFd);
}

Server::~Server() {
	stop();
}

void Server::run() {
	while (true) {
		if (poll(&pollFds[0], pollFds.size(), -1) == -1) {
			std::cerr << "Error in poll" << std::endl;
			exit(1);
		}
		for (size_t i = 0; i < pollFds.size(); ++i) {
			if (pollFds[i].revents & POLLIN) {
				if (pollFds[i].fd == serverSocket) {
					acceptConnection();
				} else {
					handleClient(pollFds[i].fd);
				}
			}
		}
	}
}

void Server::acceptConnection() {
	int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == -1) {
		std::cerr << "Error accepting connection" << std::endl;
		return;
	}
	pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	pollFds.push_back(clientPollFd);
	clients[clientSocket] = Client(clientSocket);
}

void Server::handleClient(int fd) {
	std::string request = clients[fd].receiveRequest();
	
	std::cout << "Request received:\n" << request << std::endl;

	if (request.empty()) {
		std::cerr << "Error: empty request received" << std::endl;
		close(fd);
		clients.erase(fd);
		return;
	}

	HttpRequest httpRequest(request);

	std::string body = "<html><body><h1>Welcome to WebServ!</h1></body></html>";
	HttpResponse httpResponse(200, body);
	
	std::string response = httpResponse.toString();
	std::cout << "Response being sent:\n" << response << std::endl;

	clients[fd].sendResponse(response);

	sleep(1);
	close(fd);
	clients.erase(fd);
}


void Server::stop() {
	running = false;

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
	}

	clients.clear();
	pollFds.clear();

	if (serverSocket != -1) {
		close(serverSocket);
		serverSocket = -1;
	}

	std::cout << "Server stopped safely." << std::endl;
}