#include "Server.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

Server::Server(const Config& config) {
    std::string portStr = config.getSetting("listen");
    if (!portStr.empty())
        port = std::atoi(portStr.c_str());
    else
        port = 8080; // Porta di default se la configurazione non è trovata

    std::cout << "Server configurato su porta: " << port << std::endl;
}

Server::Server(int port) : port(port) {}

std::string cleanRequest(const std::string& request) {
    size_t pos = request.find("\r\n");
    if (pos != std::string::npos) {
        return request.substr(0, pos);
    }
    return request;
}

void handleClient(int client_fd) {
    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer));
    std::string request(buffer);
    request = cleanRequest(request);
    std::cout << "Request:\n" << request << std::endl;

    std::string response;

    if (request.find("GET /upload") != std::string::npos) {
        Response res(200, "./www/upload.html");
        response = res.getResponse();
    } else if (request.find("GET / ") != std::string::npos || request.find("GET /index") != std::string::npos || request.find("GET / HTTP/1.1") != std::string::npos) {
        Response res(200, "./www/index.html");
        response = res.getResponse();
    } else {
        Response res(404, "./www/error/404.html");
        response = res.getResponse();
    }

    write(client_fd, response.c_str(), response.size());
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    _exit(0);
}

void Server::start() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);
    std::cout << "Server running on port " << port << std::endl;

    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            handleClient(client_fd);
        } else if (pid > 0) {
            close(client_fd);
            waitpid(-1, NULL, WNOHANG);
        } else {
            std::cerr << "Fork failed" << std::endl;
        }
    }
    close(server_fd);
}

