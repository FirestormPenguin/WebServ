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
	size_t _clientMaxBodySize;
	std::map<int, std::string> _errorPages;

public:
	ServerConfig();

	int getPort() const { return _port; }
	const std::string& getServerName() const { return _serverName; }
	const std::vector<LocationConfig>& getLocations() const { return _locations; }
	const std::string& getErrorPage(int code) const;
	const LocationConfig* findLocation(const std::string& path) const;
	size_t parse(const std::vector<std::string>& lines, size_t i);
	void setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
	size_t getClientMaxBodySize() const { return _clientMaxBodySize; }
	void setErrorPage(int code, const std::string& path) { _errorPages[code] = path; }
	
	
};

#endif
