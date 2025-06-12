#include "ServerConfig.hpp"
#include <sstream>
#include <iostream>

ServerConfig::ServerConfig() : _port(80) {}

int ServerConfig::getPort() const {
	return _port;
}

const std::string& ServerConfig::getServerName() const {
	return _serverName;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
	return _locations;
}

const LocationConfig* ServerConfig::findLocation(const std::string& path) const {
	(void)path;
	if (!_locations.empty())
		return &_locations[0];
	return NULL;
}

size_t ServerConfig::parse(const std::vector<std::string>& lines, size_t i) {
	for (; i < lines.size(); ++i) {
		const std::string& line = lines[i];

		if (line == "}") {
			return i + 1; // Fine del blocco server
		}
		else if (line.find("listen") == 0) {
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword >> _port;
		}
		else if (line.find("server_name") == 0) {
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword >> _serverName;
		}
		else if (line.find("location ") == 0 && line.find("{") != std::string::npos) {
			LocationConfig location;
			i = location.parse(lines, i);
			_locations.push_back(location);
		}
		else {
			std::cerr << "Invalid directive in server block: " << line << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	std::cerr << "Missing closing '}' for server block" << std::endl;
	exit(EXIT_FAILURE);
}
