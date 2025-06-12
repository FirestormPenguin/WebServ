#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include "ServerConfig.hpp"

class Config {
private:
	std::vector<ServerConfig> _servers;

public:
	Config();
	void addServer(const ServerConfig& server);
	const std::vector<ServerConfig>& getServers() const;

	void parse(const std::vector<std::string>& lines);
};

#endif
