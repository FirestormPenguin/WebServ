#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "WebServ.h"

class Client {
	public:
		Client(int socket);
		Client();
		~Client();
		std::string receiveRequest();
		void sendResponse(const std::string &response);

	private:
		int clientSocket;
};

#endif