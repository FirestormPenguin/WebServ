#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <string>

class Config
{
	private:
		std::map<std::string, std::string> settings;

	public:
		Config(const std::string &filename);
		std::string get(const std::string &key) const;
};

#endif