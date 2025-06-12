#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServ.hpp"
#include "Client.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

class Server {
private:
	ServerConfig _config;
	std::map<int, Client*> _clients;
	int _serverFd;


public:
	Server(const ServerConfig& config);
	void run();
	int getListenFd() const { return _serverFd; }
	const ServerConfig& getConfig() const { return _config; }
};

#endif
