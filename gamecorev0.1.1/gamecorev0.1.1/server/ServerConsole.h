#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include <string>
#include <vector>
#include "GameServer.h"

struct ConsoleResult
{
	std::vector<std::string> output;
	bool quit;
};

ConsoleResult runConsoleCommand(const std::string& line, GameServer& server);

#endif
