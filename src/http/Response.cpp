#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Response::Response(int code, const std::string& path) : statusCode(code) {
    std::ifstream file(path.c_str());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
    } else {
        std::cerr << "File not found: " << path << std::endl; // 🔥 Debug per capire
        content = "<h1>404 Not Found</h1>";
        statusCode = 404;
    }
}

std::string Response::getResponse() const {
    std::stringstream response;
    response << "HTTP/1.1 " << statusCode << " ";
    if (statusCode == 200)
        response << "OK\r\n";
    else
        response << "Not Found\r\n";

    response << "Content-Length: " << content.size() << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Connection: close\r\n"; // 🔥 Chiude la connessione
    response << "\r\n";
    response << content;

    return response.str();
}

