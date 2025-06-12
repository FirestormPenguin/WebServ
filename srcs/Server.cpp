#include "Server.hpp"

Server::Server(const ServerConfig& config) : _config(config) {}

void Server::run() {
	int port = _config.getPort();
	std::cout << "Launching server on port " << port << std::endl;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return;
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(sockfd);
		return;
	}

	if (listen(sockfd, 10) < 0) {
		perror("listen");
		close(sockfd);
		return;
	}

	std::cout << "Server running on port " << port << "...\n";

	// Per test: accetta un singolo client
	int client = accept(sockfd, NULL, NULL);
	if (client < 0) {
		perror("accept");
		close(sockfd);
		return;
	}

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello config!";

	send(client, response, std::strlen(response), 0);
	close(client);
	close(sockfd);
}
