#include "../includes/WebServ.h"

int main(int argc, char* argv[]) {
	int port = 8080;
	if (argc == 2)
		port = std::atoi(argv[1]);
	
	Server server(port);
	server.run();
	
	return 0;
}
