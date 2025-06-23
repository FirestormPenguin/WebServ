#include "ConfigFile.hpp"

ConfigFile::ConfigFile(const std::string& path) : _path(path) {
	readFile();
	cleanFile();
}

void ConfigFile::readFile() {
	std::ifstream file(_path.c_str());
	if (!file.is_open()) {
		std::cerr << "Error opening config file: " << _path << std::endl;
		return;
	}

	std::ostringstream ss;
	ss << file.rdbuf();
	_rawContent = ss.str();
	file.close();
}

void ConfigFile::cleanFile() {
	std::istringstream iss(_rawContent);
	std::string line;

	while (std::getline(iss, line)) {
		// Rimuovi commenti
		size_t commentPos = line.find('#');
		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos);

		// Rimuovi spazi iniziali e finali
		size_t start = line.find_first_not_of(" \t\r\n");
		size_t end = line.find_last_not_of(" \t\r\n");

		if (start != std::string::npos && end != std::string::npos)
			line = line.substr(start, end - start + 1);
		else
			line.clear();

		if (!line.empty())
			_lines.push_back(line);
	}
}

const std::vector<std::string>& ConfigFile::getLines() const {
	return _lines;
}
