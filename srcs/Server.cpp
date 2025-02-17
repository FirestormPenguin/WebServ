#include "../includes/WebServ.h"

Server::Server(int port) {
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
	close(serverSocket);
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
	HttpRequest httpRequest(request);
	HttpResponse httpResponse(200, "Hello, World!");
	clients[fd].sendResponse(httpResponse.toString());
	close(fd);
}
