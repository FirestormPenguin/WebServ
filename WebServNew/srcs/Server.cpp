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

		// Aggiungo tutti i client attivi al set di lettura
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

		// Nuova connessione in arrivo
		if (FD_ISSET(listenFd, &readFds)) {
			int newFd = accept(listenFd, NULL, NULL);
			if (newFd >= 0) {
				std::cout << "New client connected: " << newFd << std::endl;
				clients[newFd] = new Client(newFd);
			} else {
				perror("accept");
			}
		}

		// Gestione dati da client già connessi
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ) {
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
				} else {
					client->appendToRecvBuffer(std::string(buffer, bytesRead));

					// Parse richiesta HTTP minimale: estraggo il path
					std::string requestStr = client->getRecvBuffer();
					size_t posMethodEnd = requestStr.find(' ');
					size_t posPathEnd = std::string::npos;
					std::string path = "/";
					if (posMethodEnd != std::string::npos) {
						size_t posPathStart = posMethodEnd + 1;
						posPathEnd = requestStr.find(' ', posPathStart);
						if (posPathEnd != std::string::npos && posPathEnd > posPathStart) {
							path = requestStr.substr(posPathStart, posPathEnd - posPathStart);
						}
					}

					// Costruisco il path completo del file richiesto
					std::string filePath = "www" + path;
					if (filePath[filePath.length() - 1] == '/')
						filePath += "index.html";

					std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
					if (file) {
						std::ostringstream contentStream;
						contentStream << file.rdbuf();
						std::string content = contentStream.str();

						std::ostringstream oss;
						oss << "HTTP/1.1 200 OK\r\n"
							<< "Content-Type: text/html\r\n"  // TODO: gestire mime type
							<< "Content-Length: " << content.length() << "\r\n"
							<< "Connection: close\r\n"
							<< "\r\n"
							<< content;

						std::string response = oss.str();
						send(clientFd, response.c_str(), response.length(), 0);
					} else {
						std::string notFound = "<h1>404 Not Found</h1>";
						std::ostringstream oss;
						oss << "HTTP/1.1 404 Not Found\r\n"
							<< "Content-Type: text/html\r\n"
							<< "Content-Length: " << notFound.length() << "\r\n"
							<< "Connection: close\r\n"
							<< "\r\n"
							<< notFound;

						std::string response = oss.str();
						send(clientFd, response.c_str(), response.length(), 0);
					}

					// Chiudo la connessione dopo la risposta (Connection: close)
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
