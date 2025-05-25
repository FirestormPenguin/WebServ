#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "Server.hpp"
#include <sstream>
#include <string>
#include <map>

class HttpResponse
{
	public:
		HttpResponse(int statusCode, const std::string &body);
		std::string toString() const;

	private:
		int statusCode;
		std::string body;
		std::map<int, std::string> statusMessages;

		void initializeStatusMessages();
};

#endif