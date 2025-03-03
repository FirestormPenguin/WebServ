#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

class Response {
private:
    int statusCode;
    std::string content;

public:
    Response(int code, const std::string& path);
    std::string getResponse() const;

private:
    std::string getContentType(const std::string& path) const;
};

#endif

