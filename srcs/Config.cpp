#include "Config.hpp"
#include <sstream>
#include <iostream>

Config::Config() {}

void Config::addServer(const ServerConfig& server) {
	_servers.push_back(server);
}

const std::vector<ServerConfig>& Config::getServers() const {
	return _servers;
}

void Config::parse(const std::vector<std::string>& lines) {
	size_t i = 0;
	while (i < lines.size()) {
		if (lines[i] == "server {") {
			ServerConfig server;
			i = server.parse(lines, i + 1); // Passa l’indice successivo
			_servers.push_back(server);
		} else {
			std::cerr << "Invalid config: expected 'server {' but got: " << lines[i] << std::endl;
			return;
		}
	}
}
