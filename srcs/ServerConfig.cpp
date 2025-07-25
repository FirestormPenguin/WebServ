#include "ServerConfig.hpp"
#include <sstream>
#include <iostream>

ServerConfig::ServerConfig() : _port(80) {}

bool ServerConfig::isValid() const {
    if (_serverName.empty()) {
        std::cerr << "Error: server_name is required but not specified" << std::endl;
        return false;
    }
    return true;
}

const LocationConfig* ServerConfig::findLocation(const std::string& path) const {
	const LocationConfig* best = NULL;
	size_t bestLen = 0;
	for (size_t i = 0; i < _locations.size(); ++i) {
		const std::string& locPath = _locations[i].getPath();
		if (locPath == "/") {
			if (bestLen == 0) {
				best = &_locations[i];
				bestLen = 1;
			}
		} else if (path.compare(0, locPath.size(), locPath) == 0 &&
				   (path.size() == locPath.size() || path[locPath.size()] == '/')) {
			if (locPath.size() > bestLen) {
				best = &_locations[i];
				bestLen = locPath.size();
			}
		}
	}
	return best;
}

const std::string& ServerConfig::getErrorPage(int code) const {
	static const std::string empty;
	std::map<int, std::string>::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end())
		return it->second;
	return empty;
}

size_t ServerConfig::parse(const std::vector<std::string>& lines, size_t i) {
	for (; i < lines.size(); ++i) {
		const std::string& line = lines[i];

		if (line == "}") {
			return i + 1;
		}
		else if (line.find("client_max_body_size") == 0) {
			std::istringstream iss(line);
			std::string keyword;
			size_t size;
			iss >> keyword >> size;
			setClientMaxBodySize(size);
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
		else if (line.find("error_page") == 0) {
			std::istringstream iss(line);
			std::string keyword, path;
			int code;
			iss >> keyword >> code >> path;
			setErrorPage(code, path);
		}
		else {
			std::cerr << "Invalid directive in server block: " << line << std::endl;
			return lines.size();
		}
	}

	std::cerr << "Missing closing '}' for server block" << std::endl;
	return lines.size();
}
