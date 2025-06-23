#include "Server.hpp"

Server::Server(const ServerConfig& config) : _config(config) {
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0) {
		perror("socket");
		 return;
	}

	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		 return;
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_config.getPort());

	if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		 return;
	}

	if (listen(_serverFd, SOMAXCONN) < 0) {
		perror("listen");
		 return;
	}
}

void Server::run() {
	fd_set read_fds;
	int maxFd;

	while (true) {
		FD_ZERO(&read_fds);
		maxFd = _serverFd;
		FD_SET(_serverFd, &read_fds);

		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			int fd = it->first;
			FD_SET(fd, &read_fds);
			if (fd > maxFd)
				maxFd = fd;
		}

		if (select(maxFd + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			break;
		}

		// Nuova connessione
		if (FD_ISSET(_serverFd, &read_fds)) {
			int clientFd = accept(_serverFd, NULL, NULL);
			if (clientFd < 0) {
				perror("accept");
				continue;
			}
			_clients[clientFd] = new Client(clientFd);
			std::cout << "New client connected: " << clientFd << std::endl;
		}

		// Leggi dai client
		std::vector<int> toRemove;
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			int clientFd = it->first;
			Client* client = it->second;

			if (FD_ISSET(clientFd, &read_fds)) {
				char buffer[4096];
				ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

				if (bytesRead <= 0) {
					std::cout << "Client disconnected: " << clientFd << std::endl;
					close(clientFd);
					toRemove.push_back(clientFd);
					continue;
				}

				buffer[bytesRead] = '\0';
				client->appendToBuffer(buffer);

				if (client->hasCompleteRequest()) {
					client->parseRequest();
					std::string response = client->prepareResponse(_config);
					send(clientFd, response.c_str(), response.size(), 0);

					if (!client->isKeepAlive()) {
						std::cout << "Closing connection: " << clientFd << std::endl;
						close(clientFd);
						toRemove.push_back(clientFd);
					} else {
						client->reset(); // pronto a ricevere nuova richiesta
					}
				}
			}
		}

		// Rimuovi client disconnessi
		for (size_t i = 0; i < toRemove.size(); ++i) {
			delete _clients[toRemove[i]];
			_clients.erase(toRemove[i]);
		}
	}
}
