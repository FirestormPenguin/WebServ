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
	fd_set activeFds, readFds;
	int maxFd = listenFd;

	FD_ZERO(&activeFds);
	FD_SET(listenFd, &activeFds);

	while (true) {
		readFds = activeFds;

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
				FD_SET(newFd, &activeFds);
				if (newFd > maxFd)
					maxFd = newFd;
			} else {
				perror("accept");
			}
		}

		// Itera sui client
		std::map<int, Client*>::iterator it = clients.begin();
		while (it != clients.end()) {
			int clientFd = it->first;
			Client* client = it->second;

			if (FD_ISSET(clientFd, &readFds)) {
				char buffer[1024];
				std::string bufferStr;
				ssize_t bytesRead;

				do {
					memset(buffer, 0, sizeof(buffer));
					bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
					if (bytesRead > 0)
						bufferStr.append(buffer, bytesRead);
				} while (bytesRead > 0);

				if (bufferStr.empty()) {
					std::cout << "Client disconnected: " << clientFd << std::endl;
					close(clientFd);
					delete client;
					FD_CLR(clientFd, &activeFds);
					clients.erase(it++);
					continue;
				}

				client->appendToRecvBuffer(bufferStr);

				// Risposta semplice
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

				close(clientFd);
				delete client;
				FD_CLR(clientFd, &activeFds);
				clients.erase(it++);
			} else {
				++it;
			}
		}
	}
}
