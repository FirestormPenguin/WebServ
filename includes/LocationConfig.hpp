#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include "webserv.hpp"

class LocationConfig {
private:
	std::string _path;
	std::string _root;
	std::string _index;
	bool _autoindex;

public:
	LocationConfig();
	void setAutoindex(bool v) { _autoindex = v; }

	const std::string& getPath() const{ return _path; }
	const std::string& getRoot() const{ return _root; }
	const std::string& getIndex() const{ return _index; }
	bool getAutoindex() const { return _autoindex; }

	size_t parse(const std::vector<std::string>& lines, size_t i);
};

#endif
