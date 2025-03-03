#include "Server.hpp"
#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(int port) : port(port) {}

std::string cleanRequest(const std::string& request) {
    std::string clean = request;
    size_t pos = clean.find("\r\n");
    if (pos != std::string::npos) {
        clean = clean.substr(0, pos); // Prendi solo la prima riga della richiesta
    }
    return clean;
}

void Server::start() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    std::cout << "Server running on port " << port << std::endl;

    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer));
        std::string request(buffer);
        request = cleanRequest(request);
        std::cout << "Request:\n" << request << std::endl;

        std::string response;

        if (request.find("GET /404") != std::string::npos) {
            Response res(404, "./www/error/404.html");
            response = res.getResponse();
        } else if (request.find("GET /upload") != std::string::npos) {
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
    }
}

