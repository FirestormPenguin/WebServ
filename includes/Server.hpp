#ifndef SERVER_HPP
#define SERVER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Config.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <map>
#include <set>

#define MAX_CLIENTS 10
#define TIMEOUT 5000

class Client;

class Server
{
	public:
		Server(const std::string &configFile);
		~Server();
		void run();
		void shutdown();

	private:
		int serverSocket;
		int port;
		std::string serverName;
		std::string rootDir;
		std::vector<struct pollfd> pollFds;
		std::map<int, Client> clients;

		void acceptConnection();
		void handleClient(int fd);
		void closeClient(int fd);
};

#endif