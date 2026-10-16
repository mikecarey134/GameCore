#include "ServerConfig.h"
#include <fstream>
#include <sstream>

namespace
{
	const int MOST_PLAYERS = 4095;
	const char* const USAGE = "Usage: gamecore_server [--port <port>] [--max-players <count>] [--config <file>] [--smoke]";

	std::string trimmed(const std::string& text)
	{
		size_t first = text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
			return "";
		return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
	}

	bool number(const std::string& text, int lowest, int highest, int& value)
	{
		if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos || text.size() > 6)
			return false;
		value = std::stoi(text);
		return value >= lowest && value <= highest;
	}
}

bool parseServerArguments(const std::vector<std::string>& arguments, ServerSettings& settings, std::string& error)
{
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		const std::string& argument = arguments[i];
		bool hasValue = i + 1 < arguments.size();
		int value = 0;
		if (argument == "--smoke")
			settings.smoke = true;
		else if (argument == "--config" && hasValue)
			settings.configFile = arguments[++i];
		else if (argument == "--port" && hasValue && number(arguments[i + 1], 1, 65535, value))
		{
			settings.port = (uint16_t)value;
			settings.portFromArguments = true;
			++i;
		}
		else if (argument == "--max-players" && hasValue && number(arguments[i + 1], 1, MOST_PLAYERS, value))
		{
			settings.maxPlayers = value;
			settings.maxPlayersFromArguments = true;
			++i;
		}
		else
		{
			error = USAGE;
			return false;
		}
	}
	return true;
}

std::vector<std::string> applyServerConfig(const std::string& text, ServerSettings& settings)
{
	std::vector<std::string> problems;
	std::istringstream lines(text);
	std::string line;
	for (int lineNumber = 1; std::getline(lines, line); ++lineNumber)
	{
		line = trimmed(line);
		if (line.empty() || line[0] == '#')
			continue;
		size_t equals = line.find('=');
		std::string key = trimmed(line.substr(0, equals));
		std::string value = equals == std::string::npos ? "" : trimmed(line.substr(equals + 1));
		int parsed = 0;
		std::string where = "line " + std::to_string(lineNumber) + ": ";
		if (key == "port" && number(value, 1, 65535, parsed))
		{
			if (!settings.portFromArguments)
				settings.port = (uint16_t)parsed;
		}
		else if (key == "max_players" && number(value, 1, MOST_PLAYERS, parsed))
		{
			if (!settings.maxPlayersFromArguments)
				settings.maxPlayers = parsed;
		}
		else if (key == "port" || key == "max_players")
			problems.push_back(where + "bad " + key + " \"" + value + "\"");
		else
			problems.push_back(where + "unknown setting \"" + key + "\"");
	}
	return problems;
}

bool loadServerConfig(const std::string& path, ServerSettings& settings, std::vector<std::string>& problems)
{
	std::ifstream file(path.c_str());
	if (!file)
		return false;
	std::ostringstream text;
	text << file.rdbuf();
	problems = applyServerConfig(text.str(), settings);
	return true;
}
