#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <map>
#include <sys/select.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include "Client.hpp"
#include "HttpRequest.hpp"

class Server {
	private:
		int listenFd;
		std::map<int, Client*> clients;

	public:
		Server(int port);
		~Server();
		void run();
};

#endif