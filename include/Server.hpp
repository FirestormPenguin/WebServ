#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include "Config.hpp"

class Server {
private:
    int port;
    std::string serveFile(const std::string& path);
    Config config;
public:
    Server(int port);
    Server(const Config& config);
    void start();
};

#endif

