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
	fd_set writeFds;
	int maxFd;

	while (true) {
		FD_ZERO(&readFds);
		FD_ZERO(&writeFds);
		FD_SET(listenFd, &readFds);
		maxFd = listenFd;

		// Aggiungiamo tutti i client
		std::map<int, Client*>::iterator it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			int clientFd = it->first;
			FD_SET(clientFd, &readFds);
			maxFd = (clientFd > maxFd) ? clientFd : maxFd;
		}

		// Aspettiamo attività
		if (select(maxFd + 1, &readFds, NULL, NULL, NULL) < 0) {
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

		// Dati da client
		for (it = clients.begin(); it != clients.end(); ) {
			int clientFd = it->first;
			Client* client = it->second;

			if (FD_ISSET(clientFd, &readFds)) {
				char buffer[4096];
				ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

				if (bytesRead <= 0) {
					std::cout << "Closing client " << clientFd << std::endl;
					close(clientFd);
					delete client;
					clients.erase(it++);
					continue;
				} else {
					client->appendToRecvBuffer(std::string(buffer, bytesRead));

					std::string body = "Hello World\n";
					std::ostringstream oss;
					oss << "HTTP/1.1 200 OK\r\n"
						<< "Content-Type: text/plain\r\n"
						<< "Content-Length: " << body.length() << "\r\n"
						<< "Connection: close\r\n"
						<< "\r\n"
						<< body;
					std::string response = oss.str();

					// Invio della risposta completa
					send(clientFd, response.c_str(), response.size(), 0);

					// Chiudo il client
					std::cout << "Client " << clientFd << " served, closing." << std::endl;
					close(clientFd);
					delete client;
					clients.erase(it++);
					continue;
				}
			}
			++it;
		}
	}
}
