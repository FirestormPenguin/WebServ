#include "../includes/Config.hpp"

Config::Config(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	std::string line;

	if (!file.is_open())
	{
		std::cerr << "Error opening config file: " << filename << std::endl;
		return;
	}
	
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string key, value;

		if (line.empty() || line[0] == '#')
			continue;
		
		// size_t delimiter = line.find('=');
		// if (delimiter != std::string::npos)
		// {
		// 	key = line.substr(0, delimiter);
		// 	value = line.substr(delimiter + 1);
		// 	settings[key] = value;
		// }
		ss >> key;
		if (ss >> value)
		{
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
