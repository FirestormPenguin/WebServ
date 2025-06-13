#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include "webserv.hpp"

class LocationConfig {
private:
	std::string _path;
	std::string _root;
	std::string _index;
	std::string _redirectUrl;
	int _redirectCode;
	bool _autoindex;
	std::set<std::string> _allowedMethods;
	std::map<std::string, std::string> _cgiMap;
	

public:
	LocationConfig();
	void setAutoindex(bool v) { _autoindex = v; }
	void setRedirect(int code, const std::string& url) { _redirectCode = code; _redirectUrl = url; }
	void addAllowedMethod(const std::string& m) { _allowedMethods.insert(m); }
	void addCgi(const std::string& ext, const std::string& path) { _cgiMap[ext] = path; }
	
	const std::string& getPath() const{ return _path; }
	const std::string& getRoot() const{ return _root; }
	const std::string& getIndex() const{ return _index; }
	const std::string& getRedirectUrl() const { return _redirectUrl; }
	int getRedirectCode() const { return _redirectCode; }
	bool getAutoindex() const { return _autoindex; }
	bool isMethodAllowed(const std::string& m) const { return _allowedMethods.count(m) > 0; }
	const std::map<std::string, std::string>& getCgiMap() const { return _cgiMap; }
	
	size_t parse(const std::vector<std::string>& lines, size_t i);
};

#endif
