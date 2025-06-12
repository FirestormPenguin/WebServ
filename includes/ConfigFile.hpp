#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Server.hpp"

class ConfigFile {
private:
	std::string _path;
	std::string _rawContent;
	std::vector<std::string> _lines;

	void readFile();
	void cleanFile();

public:
	ConfigFile(const std::string& path);
	const std::vector<std::string>& getLines() const;
};

#endif
