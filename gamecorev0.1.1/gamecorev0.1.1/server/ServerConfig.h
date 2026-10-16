#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <cstdint>
#include <string>
#include <vector>

struct ServerSettings
{
	uint16_t port = 1080;
	int maxPlayers = 8;
	std::string configFile = "server.cfg";
	bool smoke = false;
	bool portFromArguments = false;
	bool maxPlayersFromArguments = false;
};

bool parseServerArguments(const std::vector<std::string>& arguments, ServerSettings& settings, std::string& error);
std::vector<std::string> applyServerConfig(const std::string& text, ServerSettings& settings);
bool loadServerConfig(const std::string& path, ServerSettings& settings, std::vector<std::string>& problems);

#endif
