#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

Response::Response(int code, const std::string& path) : statusCode(code), content(readFile(path)) {}

std::string Response::getStatusMessage(int code) const {
    if (code == 200) return "OK";
    if (code == 404) return "Not Found";
    return "Unknown";
}

std::string Response::getContentType(const std::string& path) const {
    std::map<std::string, std::string> contentTypes;
    contentTypes[".html"] = "text/html";
    contentTypes[".css"] = "text/css";
    contentTypes[".js"] = "application/javascript";
    contentTypes[".png"] = "image/png";
    contentTypes[".jpg"] = "image/jpeg";
    contentTypes[".jpeg"] = "image/jpeg";
    contentTypes[".gif"] = "image/gif";

    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = path.substr(pos);
        if (contentTypes.find(ext) != contentTypes.end()) {
            return contentTypes[ext];
        }
    }
    return "application/octet-stream";
}

std::string Response::readFile(const std::string& path) const {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        return ""; // Ritornare stringa vuota per evitare problemi di download
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string Response::getResponse() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
    response << "Content-Type: " << getContentType("index.html") << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Connection: close\r\n"; // Chiudere la connessione per evitare lentezza
    response << "\r\n";
    response << content;
    return response.str();
}

