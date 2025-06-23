#include "Server.hpp"
#include "ConfigFile.hpp"
#include "Config.hpp"
#include <set>

int main(int argc, char** argv) {
	std::string configPath = "webserv.conf";
	if (argc == 2)
		configPath = argv[1];

	ConfigFile confFile(configPath);
	Config config;
	config.parse(confFile.getLines());

	const std::vector<ServerConfig>& servers = config.getServers();
	std::vector<Server*> serverList;
	std::map<int, Server*> fdToServer; // Map from listen fd to Server*

	// Crea tutti i server e raccogli solo quelli con fd valido
	for (size_t i = 0; i < servers.size(); ++i) {
		Server* s = new Server(servers[i]);
		int fd = s->getListenFd();
		if (fd >= 0) {
			serverList.push_back(s);
			fdToServer[fd] = s;
			std::cout << "Listening on port " << servers[i].getPort() << std::endl;
		} else {
			delete s;
			std::cerr << "Server on port " << servers[i].getPort() << " not started due to socket/bind/listen error." << std::endl;
		}
	}

	std::map<int, Client*> clients;
	std::map<int, int> clientToServerFd; // Mappa client fd -> listen fd

	while (true) {
		fd_set read_fds;
		FD_ZERO(&read_fds);

		int maxFd = 0;

		// Aggiungi solo i socket di ascolto validi
		for (size_t i = 0; i < serverList.size(); ++i) {
			int fd = serverList[i]->getListenFd();
			if (fd >= 0) {
				FD_SET(fd, &read_fds);
				if (fd > maxFd) maxFd = fd;
			}
		}

		// Aggiungi tutti i socket dei client
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
			FD_SET(it->first, &read_fds);
			if (it->first > maxFd) maxFd = it->first;
		}

		if (select(maxFd + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			break;
		}

		// Accetta nuove connessioni solo su fd validi
		for (size_t i = 0; i < serverList.size(); ++i) {
			int listenFd = serverList[i]->getListenFd();
			if (listenFd >= 0 && FD_ISSET(listenFd, &read_fds)) {
				int clientFd = accept(listenFd, NULL, NULL);
				if (clientFd < 0) {
					perror("accept");
					continue;
				}
				clients[clientFd] = new Client(clientFd);
				clientToServerFd[clientFd] = listenFd;
				std::cout << "New client connected: " << clientFd << std::endl;
			}
		}

		// Gestisci le richieste dei client
		std::vector<int> toRemove;
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
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
					int listenFd = clientToServerFd[clientFd];
					Server* server = fdToServer[listenFd];
					std::string response = client->prepareResponse(server->getConfig());
					send(clientFd, response.c_str(), response.size(), 0);

					if (!client->isKeepAlive()) {
						std::cout << "Closing connection: " << clientFd << std::endl;
						close(clientFd);
						toRemove.push_back(clientFd);
					} else {
						client->reset();
					}
				}
			}
		}

		// Rimuovi i client disconnessi
		for (size_t i = 0; i < toRemove.size(); ++i) {
			delete clients[toRemove[i]];
			clients.erase(toRemove[i]);
			clientToServerFd.erase(toRemove[i]);
		}
	}

	// Cleanup
	for (size_t i = 0; i < serverList.size(); ++i) {
		delete serverList[i];
	}
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		delete it->second;
	}
	return 0;
}