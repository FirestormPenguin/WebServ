#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

class Client
{
	public:
		Client(int socket);
		
		~Client();
		
		std::string receiveRequest();
		int sendResponse(const std::string &response);
		
	private:
		int clientSocket;
};

#endif
