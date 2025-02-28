#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <poll.h>

#define MAX_CLIENTS 10
#define TIMEOUT 5000

class Server
{
	public:
		Server(int port);
		~Server();
		void run();

	private:
		int serverSocket;
		std::vector<struct pollfd> clients;

		void createSocket();
		void bindSocket(int port);
		void listenSocket();
		void handleNewConnection();
		void handleClient(size_t index);
		void removeClient(size_t index);
};

#endif