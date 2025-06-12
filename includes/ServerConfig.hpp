#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include "LocationConfig.hpp"

class ServerConfig {
private:
	int _port;
	std::string _serverName;
	std::vector<LocationConfig> _locations;

public:
	ServerConfig();

	int getPort() const;
	const std::string& getServerName() const;
	const std::vector<LocationConfig>& getLocations() const;

	size_t parse(const std::vector<std::string>& lines, size_t i);
};

#endif
