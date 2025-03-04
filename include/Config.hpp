#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

class Config {
private:
    std::map<std::string, std::string> settings;

public:
    Config();
    Config(const std::string& filename);
    std::string getSetting(const std::string& key) const;
    void printConfig() const;
};

#endif

