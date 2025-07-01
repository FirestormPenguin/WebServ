#include "LocationConfig.hpp"
#include <sstream>
#include <iostream>

LocationConfig::LocationConfig() { _redirectCode = 0; _redirectUrl = ""; }

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
		else if (line.find("autoindex") == 0) {
			std::istringstream iss(line);
			std::string keyword, value;
			iss >> keyword >> value;
			setAutoindex(value == "on");
		}
		else if (line.find("return") == 0) {
			std::istringstream iss(line);
			std::string keyword, url;
			int code;
			iss >> keyword >> code >> url;
			setRedirect(code, url);
		}
		else if (line.find("allow_methods") == 0) {
			std::istringstream iss(line);
			std::string keyword, method;
			iss >> keyword;
			while (iss >> method)
				addAllowedMethod(method);
		}
		else if (line.find("cgi") == 0) {
			std::istringstream iss(line);
			std::string keyword, ext, path;
			iss >> keyword >> ext >> path;
			addCgi(ext, path);
		}
		else {
			std::cerr << "Invalid directive in location block: " << line << std::endl;
			return lines.size();
		}
	}

	std::cerr << "Missing closing '}' for location block" << std::endl;
	return lines.size();
}
