#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include "Client.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

class Server {
private:
	ServerConfig _config;

public:
	Server(const ServerConfig& config);
	void run();
};

#endif
