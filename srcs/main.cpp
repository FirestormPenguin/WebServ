#include "../includes/WebServ.h"
#include "../includes/Server.hpp"
#include <csignal>

Server* serverInstance = NULL;

void signalHandler(int signum) {
	if (serverInstance) {
		std::cout << "\nStopping server..." << std::endl;
		serverInstance->stop();
	}
	exit(signum);
}

int main() {
	Server server(8080);
	serverInstance = &server;

	signal(SIGINT, signalHandler);
	std::cout << "\nServer running..." << std::endl;
	server.run();

	return 0;
}
