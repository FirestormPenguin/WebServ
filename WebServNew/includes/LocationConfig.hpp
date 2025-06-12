#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include "webserv.hpp"

class LocationConfig {
private:
	std::string _path;
	std::string _root;
	std::string _index;

public:
	LocationConfig();

	const std::string& getPath() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;

	size_t parse(const std::vector<std::string>& lines, size_t i);
};

#endif
