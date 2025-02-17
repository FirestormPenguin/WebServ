#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServ.h"
#include "Client.hpp"

class Server {
	public:
		Server(int port);
		~Server();
		void run();

	private:
		int serverSocket;
		std::vector<struct pollfd> pollFds;
		std::map<int, Client> clients;
		void acceptConnection();
		void handleClient(int fd);
};

#endif