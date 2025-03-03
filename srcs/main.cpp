#include "../includes/Server.hpp"
#include <iostream>
#include <csignal>

Server* globalServer = NULL;

void handleSignal(int signal)
{
	signal  = 0;
	if (globalServer)
	{
		std::cout << "\nShutting down server gracefully..." << std::endl;
		globalServer->shutdown();
		exit(0);
	}
}

int main()
{
	std::signal(SIGINT, handleSignal);
	std::signal(SIGTERM, handleSignal);

	try
	{
		Server server("config.conf");
		globalServer = &server;
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
