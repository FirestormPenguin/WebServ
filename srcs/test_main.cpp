#include "ConfigFile.hpp"
#include "Config.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "Client.hpp"
#include <iostream>
#include <vector>
#include <string>

void test_request(const ServerConfig& config, const std::string& rawRequest, const std::string& testName) {
	Client fakeClient(0); // fd non usato
	fakeClient.appendToBuffer(rawRequest);
	fakeClient.parseRequest();
	std::string response = fakeClient.prepareResponse(config);

	std::cout << "=== " << testName << " ===" << std::endl;
	std::cout << response << std::endl;
	std::cout << "==========================" << std::endl << std::endl;
}

int main() {
	// Carica la config reale
	ConfigFile confFile("webserv.conf");
	Config config;
	config.parse(confFile.getLines());
	const ServerConfig& server = config.getServers()[0];

	// 1. GET root (index.html)
	test_request(server,
		"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"GET / (index.html)");

	// 2. GET file che non esiste (404)
	test_request(server,
		"GET /notfound.html HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"GET /notfound.html (404)");

	// 3. GET directory con autoindex
	test_request(server,
		"GET /testdir/ HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"GET /testdir/ (autoindex)");

	// 4. POST (upload)
	test_request(server,
		"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 11\r\n\r\nHello World",
		"POST / (upload)");

	// 5. DELETE file (prova a cancellare un file esistente)
	test_request(server,
		"DELETE /upload.data HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"DELETE /upload.data");

	// 6. Metodo non permesso (PUT)
	test_request(server,
		"PUT / HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"PUT / (405)");

	// 7. Body troppo grande (413)
	test_request(server,
		"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1000001\r\n\r\n" + std::string(1000001, 'A'),
		"POST / (413 Payload Too Large)");

	// 8. CGI PHP
	test_request(server,
		"GET /cgi-bin/test.php HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"GET /cgi-bin/test.php (CGI PHP)");

	// 9. CGI Python
	test_request(server,
		"GET /cgi-bin/test.py HTTP/1.1\r\nHost: localhost\r\n\r\n",
		"GET /cgi-bin/test.py (CGI Python)");

	// 10. Redirezione (decommenta la location return nel .conf per testarla)
	// test_request(server,
	//     "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
	//     "GET / (redirect)");

	return 0;
}