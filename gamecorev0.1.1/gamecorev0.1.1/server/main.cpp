#include <chrono>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "GameServer.h"
#include "ServerConfig.h"
#include "ServerConsole.h"
#include "net/EnetTransport.h"

namespace
{
	std::mutex consoleLock;
	std::deque<std::string> consoleLines;

	void readConsole()
	{
		std::string line;
		while (std::getline(std::cin, line))
		{
			std::lock_guard<std::mutex> guard(consoleLock);
			consoleLines.push_back(line);
		}
	}

	bool nextConsoleLine(std::string& line)
	{
		std::lock_guard<std::mutex> guard(consoleLock);
		if (consoleLines.empty())
			return false;
		line = consoleLines.front();
		consoleLines.pop_front();
		return true;
	}
}

int main(int argc, char** argv)
{
	ServerSettings settings;
	std::string error;
	if (!parseServerArguments(std::vector<std::string>(argv + 1, argv + argc), settings, error))
	{
		std::cerr << error << std::endl;
		return 2;
	}
	std::vector<std::string> problems;
	if (loadServerConfig(settings.configFile, settings, problems))
		std::cout << "Read " << settings.configFile << std::endl;
	else
		std::cout << "No " << settings.configFile << ", using defaults" << std::endl;
	for (size_t i = 0; i < problems.size(); ++i)
		std::cerr << settings.configFile << " " << problems[i] << std::endl;

	std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
	EnetTransport transport;
	GameServer server(transport,
		[started] { return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count(); },
		[](const std::string& line) { std::cout << line << std::endl; });

	if (!server.start(settings.port, settings.maxPlayers))
	{
		std::cerr << "Could not listen on port " << settings.port << std::endl;
		return 1;
	}
	std::cout << "GameCore server listening on port " << settings.port << " for up to " << settings.maxPlayers << " players" << std::endl;

	if (settings.smoke)
	{
		server.tick();
		server.stop();
		std::cout << "Server smoke run finished" << std::endl;
		return 0;
	}

	std::thread(readConsole).detach();
	bool running = true;
	while (running)
	{
		server.tick();
		std::string line;
		while (running && nextConsoleLine(line))
		{
			ConsoleResult result = runConsoleCommand(line, server);
			for (size_t i = 0; i < result.output.size(); ++i)
				std::cout << result.output[i] << std::endl;
			running = !result.quit;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	server.stop();
	std::cout << "Server stopped" << std::endl;
	std::_Exit(0);
}
