#include "LocationConfig.hpp"
#include <sstream>
#include <iostream>

LocationConfig::LocationConfig() {}

const std::string& LocationConfig::getPath() const { return _path; }
const std::string& LocationConfig::getRoot() const { return _root; }
const std::string& LocationConfig::getIndex() const { return _index; }

size_t LocationConfig::parse(const std::vector<std::string>& lines, size_t i) {
	std::istringstream iss(lines[i]);
	std::string keyword;
	iss >> keyword >> _path;

	++i; // skip "location /path {"
	for (; i < lines.size(); ++i) {
		const std::string& line = lines[i];
		if (line == "}") return i;
		if (line.find("root") == 0) {
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword >> _root;
		}
		else if (line.find("index") == 0) {
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword >> _index;
		}
		else {
			std::cerr << "Invalid directive in location block: " << line << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	std::cerr << "Missing closing '}' for location block" << std::endl;
	exit(EXIT_FAILURE);
}
