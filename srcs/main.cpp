#include "Server.hpp"
#include "ConfigFile.hpp"
#include "Config.hpp"

int main(int argc, char** argv) {
	std::string configPath = "webserv.conf";
	if (argc == 2)
		configPath = argv[1];

	ConfigFile confFile(configPath);
	Config config;
	config.parse(confFile.getLines());

	const std::vector<ServerConfig>& servers = config.getServers();
	for (size_t i = 0; i < servers.size(); ++i) {
		Server s(servers[i]);
		s.run();
	}
	return 0;
}
