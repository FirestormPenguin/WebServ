#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "WebServ.h"

class HttpResponse {
	public:
		HttpResponse(int statusCode, const std::string &body);
		std::string toString() const;

	private:
		int statusCode;
		std::string body;
};

#endif