#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

class Response {
private:
    int statusCode;
    std::string content;
    std::string filePath;

    std::string getStatusMessage(int code) const;
    std::string getContentType(const std::string& path) const;
    std::string readFile(const std::string& path) const;

public:
    Response(int code, const std::string& path);
    std::string getResponse() const;
};

#endif

