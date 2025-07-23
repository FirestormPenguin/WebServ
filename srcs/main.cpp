#include "Server.hpp"
#include "ConfigFile.hpp"
#include "Config.hpp"
#include <set>
#include <map>
#include <sstream>
#include <iostream>

std::string extractHost(const std::string& rawRequest) {
	std::istringstream iss(rawRequest);
	std::string line;
	while (std::getline(iss, line)) {
		if (line.find("Host:") == 0) {
			std::string host = line.substr(5);
			size_t pos = host.find(':');
			if (pos != std::string::npos)
				host = host.substr(0, pos);
			while (!host.empty() && (host[0] == ' ' || host[0] == '\t'))
				host.erase(0, 1);
			while (!host.empty() && (host[host.size()-1] == '\r' || host[host.size()-1] == ' ' || host[host.size()-1] == '\t'))
				host.erase(host.size()-1, 1);
			return host;
		}
	}
	return "";
}

int main(int argc, char** argv) {
	std::string configPath = "webserv.conf";
	if (argc == 2)
		configPath = argv[1];

	ConfigFile confFile(configPath);
	Config config;
	config.parse(confFile.getLines());

	const std::vector<ServerConfig>& servers = config.getServers();

	// Validazione dei server prima di avviarli
    for (size_t i = 0; i < servers.size(); ++i) {
        if (!servers[i].isValid()) {
            std::cerr << "Server configuration invalid. Exiting." << std::endl;
            return 1;
        }
    }

	std::vector<Server*> serverList;
	std::map<int, Server*> fdToServer; // Map from listen fd to Server*

	// Raggruppa i server per porta per il virtual hosting
	std::map<int, std::vector<const ServerConfig*> > serversByPort;
	for (size_t i = 0; i < servers.size(); ++i) {
		serversByPort[servers[i].getPort()].push_back(&servers[i]);
	}

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
			return 1;
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
				char buffer[65536];  // Buffer più grande per file grandi
				ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

				if (bytesRead <= 0) {
					std::cout << "Client disconnected: " << clientFd << std::endl;
					close(clientFd);
					toRemove.push_back(clientFd);
					continue;
				}

				// Per dati binari, usa std::string con lunghezza esplicita
				std::string received_data(buffer, bytesRead);
				client->appendToBuffer(received_data);

				// Controlla se il client aspetta "100 Continue"
				std::string recvBuffer = client->getRecvBuffer();
				if (recvBuffer.find("Expect: 100-continue") != std::string::npos && 
					!client->hasSent100Continue()) {
					std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
					send(clientFd, continueResponse.c_str(), continueResponse.size(), 0);
					client->setSent100Continue(true);
				} 
				// CASO SPECIALE: Multipart senza "Expect: 100-continue" (Chrome moderno)
				else if (recvBuffer.find("multipart/form-data") != std::string::npos && 
						 recvBuffer.find("\r\n\r\n") != std::string::npos &&
						 !client->hasSent100Continue()) {
					std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
					send(clientFd, continueResponse.c_str(), continueResponse.size(), 0);
					client->setSent100Continue(true);
				}

				// Dopo aver inviato 100 Continue, prova a leggere immediatamente più dati
				if (recvBuffer.find("multipart/form-data") != std::string::npos && client->hasSent100Continue()) {
					fd_set immediate_read;
					struct timeval immediate_timeout;
					FD_ZERO(&immediate_read);
					FD_SET(clientFd, &immediate_read);
					immediate_timeout.tv_sec = 0;
					immediate_timeout.tv_usec = 100000; // 100ms timeout
					
					int immediate_ready = select(clientFd + 1, &immediate_read, NULL, NULL, &immediate_timeout);
					if (immediate_ready > 0 && FD_ISSET(clientFd, &immediate_read)) {
						char immediate_buffer[65536];
						ssize_t immediate_bytes = recv(clientFd, immediate_buffer, sizeof(immediate_buffer) - 1, MSG_DONTWAIT);
						if (immediate_bytes > 0) {
							std::string immediate_data(immediate_buffer, immediate_bytes);
							client->appendToBuffer(immediate_data);
						}
					}
				}

				if (client->hasCompleteRequest()) {
					client->parseRequest();
					int listenFd = clientToServerFd[clientFd];

					// Virtual host selection strict
					int port = 0;
					for (size_t i = 0; i < serverList.size(); ++i) {
						if (serverList[i]->getListenFd() == listenFd) {
							port = serverList[i]->getConfig().getPort();
							break;
						}
					}
					std::string hostHeader = extractHost(client->getRecvBuffer());
					const std::vector<const ServerConfig*>& candidates = serversByPort[port];
					const ServerConfig* selected = NULL;
					for (size_t i = 0; i < candidates.size(); ++i) {
						if (candidates[i]->getServerName() == hostHeader) {
							selected = candidates[i];
							break;
						}
					}
					if (!selected) {
						std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
						send(clientFd, response.c_str(), response.size(), 0);
						close(clientFd);
						toRemove.push_back(clientFd);
						continue;
					}

					std::string response = client->prepareResponse(*selected);
					
					// Se la risposta è vuota, significa che stiamo aspettando più dati
					// (probabilmente dopo aver inviato "100 Continue")
					if (!response.empty()) {
						send(clientFd, response.c_str(), response.size(), 0);

						if (!client->isKeepAlive()) {
							std::cout << "Closing connection: " << clientFd << std::endl;
							close(clientFd);
							toRemove.push_back(clientFd);
						} else {
							client->reset();
						}
					} else {
						std::cerr << "Empty response - waiting for more data from client " << clientFd << std::endl;
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
