#include "ServerConsole.h"

namespace
{
	std::string trimmed(const std::string& text)
	{
		size_t first = text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
			return "";
		return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
	}
}

ConsoleResult runConsoleCommand(const std::string& line, GameServer& server)
{
	ConsoleResult result = ConsoleResult{ std::vector<std::string>(), false };
	std::string text = trimmed(line);
	if (text.empty())
		return result;
	size_t space = text.find(' ');
	std::string command = text.substr(0, space);
	std::string argument = space == std::string::npos ? "" : trimmed(text.substr(space + 1));

	if (command == "say")
	{
		if (argument.empty())
			result.output.push_back("Usage: say <text>");
		else
			server.say(argument);
	}
	else if (command == "kick")
	{
		if (server.kick(argument))
			result.output.push_back("Kicked " + argument);
		else
			result.output.push_back("No player named " + argument);
	}
	else if (command == "list")
	{
		result.output = server.playerList();
		if (result.output.empty())
			result.output.push_back("No players");
	}
	else if (command == "quit")
		result.quit = true;
	else
		result.output.push_back("Unknown command " + command + ". Commands: say <text>, kick <name>, list, quit");
	return result;
}
