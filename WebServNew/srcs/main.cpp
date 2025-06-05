#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <port>" << std::endl;
		return 1;
	}

	int port = atoi(argv[1]);
	Server server(port);
	server.run();

	return 0;
}
