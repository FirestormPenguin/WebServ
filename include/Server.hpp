#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>

class Server {
private:
    int port;
    std::string serveFile(const std::string& path);
public:
    Server(int port);
    void start();
};

#endif

