#include "Server.hpp"

Server::Server(int port) {
	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(listenFd, SOMAXCONN) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "Server started on port " << port << std::endl;
}

Server::~Server() {
	close(listenFd);
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
		delete it->second;
	}
	clients.clear();
}

void Server::run() {
	fd_set readFds;
	int maxFd;

	while (true) {
		FD_ZERO(&readFds);
		FD_SET(listenFd, &readFds);
		maxFd = listenFd;

		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
			int clientFd = it->first;
			FD_SET(clientFd, &readFds);
			if (clientFd > maxFd)
				maxFd = clientFd;
		}

		int activity = select(maxFd + 1, &readFds, NULL, NULL, NULL);
		if (activity < 0) {
			perror("select");
			break;
		}

		// Nuova connessione
		if (FD_ISSET(listenFd, &readFds)) {
			int newFd = accept(listenFd, NULL, NULL);
			if (newFd >= 0) {
				std::cout << "New client connected: " << newFd << std::endl;
				clients[newFd] = new Client(newFd);
			} else {
				perror("accept");
			}
		}

		// Dati dai client
		std::map<int, Client*>::iterator it = clients.begin();
		while (it != clients.end()) {
			int clientFd = it->first;
			Client* client = it->second;

			if (FD_ISSET(clientFd, &readFds)) {
				char buffer[1025]; // +1 per il null terminator
				ssize_t bytesRead = recv(clientFd, buffer, 1024, 0);

				if (bytesRead <= 0) {
					std::cout << "Closing client " << clientFd << std::endl;
					close(clientFd);
					delete client;
					clients.erase(it++);
				} else {
					buffer[bytesRead] = '\0';
					client->appendToRecvBuffer(std::string(buffer, bytesRead));

					// Parsing della richiesta HTTP
					HttpRequest request(client->getRecvBuffer());

					std::cout << "=== HTTP Request Received ===" << std::endl;
					std::cout << "Method: " << request.getMethod() << std::endl;
					std::cout << "Path: " << request.getPath() << std::endl;
					std::cout << "Version: " << request.getVersion() << std::endl;

					const std::map<std::string, std::string>& headers = request.getHeaders();
					for (std::map<std::string, std::string>::const_iterator hit = headers.begin(); hit != headers.end(); ++hit) {
						std::cout << hit->first << ": " << hit->second << std::endl;
					}

					// Risposta semplice per test
					std::string body = "Hello World\r\n";
					std::ostringstream oss;
					oss << "HTTP/1.1 200 OK\r\n"
						<< "Content-Type: text/plain\r\n"
						<< "Content-Length: " << body.length() << "\r\n"
						<< "Connection: close\r\n"
						<< "\r\n"
						<< body;
					std::string response = oss.str();
					send(clientFd, response.c_str(), response.length(), 0);

					std::cout << "Client " << clientFd << " served, closing." << std::endl;
					close(clientFd);
					delete client;
					clients.erase(it++);
				}
			} else {
				++it;
			}
		}
	}
}

