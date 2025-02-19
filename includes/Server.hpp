#ifndef SERVER_HPP
#define SERVER_HPP

# include "WebServ.h"
# include "Client.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class Server {
	public:
		Server(int port);
		~Server();
		void run();
		void stop();
	private:
		int serverSocket;
		bool running;
		std::vector<struct pollfd> pollFds;
		std::map<int, Client> clients;

		void acceptConnection();
		void handleClient(int fd);
};

#endif
