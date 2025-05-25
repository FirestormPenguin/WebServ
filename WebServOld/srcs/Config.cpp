#include "../includes/Config.hpp"

Config::Config(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error opening config file: " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		
		size_t delimiter = line.find('=');
		if (delimiter != std::string::npos)
		{
			std::string key = line.substr(0, delimiter);
			std::string value = line.substr(delimiter + 1);
			settings[key] = value;
		}
	}
	file.close();
}

std::string Config::get(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = settings.find(key);
	if (it != settings.end())
	{
		return it->second;
	}
	return "";
}
