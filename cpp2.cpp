#include <iostream>
#include <map>
#include <vector>
#include <cstring>

void parse_http_request(const std::string& request) {
	std::vector<std::string> lines;
	size_t start = 0;
	size_t end = request.find("\r\n");

	// Dividiamo la richiesta in righe
	while (end != std::string::npos) {
		lines.push_back(request.substr(start, end - start));
		start = end + 2; // Saltiamo "\r\n"
		end = request.find("\r\n", start);
	}

	if (lines.empty())
		return;

	// Analizziamo la prima riga
	std::vector<std::string> first_line;
	std::istringstream first_stream(lines[0]);
	std::string word;
	while (first_stream >> word) {
		first_line.push_back(word);
	}

	if (first_line.size() < 3)
		return;

	std::string method = first_line[0];
	std::string path = first_line[1];
	std::string version = first_line[2];

	std::cout << "Metodo: " << method << "\n";
	std::cout << "Percorso: " << path << "\n";
	std::cout << "Versione: " << version << "\n";

	// Analizziamo gli header
	std::map<std::string, std::string> headers;
	for (size_t i = 1; i < lines.size(); i++) {
		size_t pos = lines[i].find(": ");
		if (pos != std::string::npos) {
			std::string key = lines[i].substr(0, pos);
			std::string value = lines[i].substr(pos + 2);
			headers[key] = value;
		}
	}

	std::cout << "Headers:\n";
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << "\n";
	}
}

int main() {
	std::string request =
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"User-Agent: Mozilla/5.0\r\n"
		"Accept: text/html\r\n"
		"\r\n";

	parse_http_request(request);
	return 0;
}
