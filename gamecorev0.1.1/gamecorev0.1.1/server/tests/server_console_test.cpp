#include "doctest.h"
#include "server/ServerConsole.h"
#include "net/LoopbackTransport.h"

using namespace protocol;

namespace
{
	struct ConsoleFixture
	{
		ConsoleFixture()
			: serverTransport(network), client(network),
			  server(serverTransport, [] { return 1000u; }, [](const std::string&) {})
		{
			REQUIRE(server.start(GameServer::DEFAULT_PORT));
		}

		void joinAs(const std::string& name)
		{
			REQUIRE(client.connect("localhost", GameServer::DEFAULT_PORT));
			NetEvent event;
			PeerId serverPeer = NO_PEER;
			while (client.poll(event))
				if (event.kind == NetEvent::Connected)
					serverPeer = event.peer;
			client.send(serverPeer, encode(HELLO, Hello{ PROTOCOL_VERSION, name, 0 }), Reliable);
			server.tick();
		}

		std::vector<uint8_t> nextTypes()
		{
			server.tick();
			std::vector<uint8_t> types;
			NetEvent event;
			while (client.poll(event))
			{
				if (event.kind == NetEvent::Disconnected)
					types.push_back(255);
				std::vector<Frame> frames;
				if (event.kind == NetEvent::Received && unframe(event.data, frames))
					for (size_t i = 0; i < frames.size(); ++i)
					{
						types.push_back(frames[i].type);
						ByteReader in(frames[i].payload, frames[i].size);
						ServerText text;
						if (frames[i].type == SERVER_TEXT && read(in, text))
							lastText = text.text;
					}
			}
			return types;
		}

		LoopbackNetwork network;
		LoopbackTransport serverTransport;
		LoopbackTransport client;
		GameServer server;
		std::string lastText;
	};
}

TEST_CASE_FIXTURE(ConsoleFixture, "say sends the rest of the line to every player")
{
	joinAs("mikey");
	nextTypes();
	ConsoleResult result = runConsoleCommand("say  hello there\r", server);
	CHECK_FALSE(result.quit);
	std::vector<uint8_t> types = nextTypes();
	REQUIRE(types == std::vector<uint8_t>{ SERVER_TEXT });
	CHECK(lastText == "hello there");
}

TEST_CASE_FIXTURE(ConsoleFixture, "say with nothing to say prints its usage")
{
	joinAs("mikey");
	nextTypes();
	ConsoleResult result = runConsoleCommand("say", server);
	REQUIRE(result.output.size() == 1);
	CHECK(result.output[0].find("say <text>") != std::string::npos);
	CHECK(nextTypes().empty());
}

TEST_CASE_FIXTURE(ConsoleFixture, "kick removes a player by name and reports an unknown one")
{
	joinAs("mikey");
	nextTypes();
	ConsoleResult missing = runConsoleCommand("kick dan", server);
	REQUIRE(missing.output.size() == 1);
	CHECK(missing.output[0].find("dan") != std::string::npos);
	CHECK(nextTypes().empty());

	ConsoleResult kicked = runConsoleCommand("kick mikey\r", server);
	REQUIRE(kicked.output.size() == 1);
	CHECK(kicked.output[0].find("mikey") != std::string::npos);
	CHECK(nextTypes() == std::vector<uint8_t>{ KICK, 255 });
}

TEST_CASE_FIXTURE(ConsoleFixture, "list prints each player or says there are none")
{
	ConsoleResult empty = runConsoleCommand("list", server);
	REQUIRE(empty.output.size() == 1);
	CHECK(empty.output[0] == "No players");

	joinAs("mikey");
	ConsoleResult one = runConsoleCommand("list", server);
	CHECK(one.output == std::vector<std::string>{ "1 mikey" });
}

TEST_CASE_FIXTURE(ConsoleFixture, "quit asks the server loop to stop")
{
	CHECK(runConsoleCommand("quit", server).quit);
	CHECK(runConsoleCommand("  quit  ", server).quit);
	CHECK_FALSE(runConsoleCommand("list", server).quit);
}

TEST_CASE_FIXTURE(ConsoleFixture, "an unknown command names the commands that exist")
{
	ConsoleResult result = runConsoleCommand("explode now", server);
	CHECK_FALSE(result.quit);
	REQUIRE(result.output.size() == 1);
	CHECK(result.output[0].find("explode") != std::string::npos);
	CHECK(result.output[0].find("say") != std::string::npos);
	CHECK(result.output[0].find("kick") != std::string::npos);
	CHECK(result.output[0].find("list") != std::string::npos);
	CHECK(result.output[0].find("quit") != std::string::npos);
}

TEST_CASE_FIXTURE(ConsoleFixture, "a blank line does nothing")
{
	ConsoleResult result = runConsoleCommand("   \r", server);
	CHECK(result.output.empty());
	CHECK_FALSE(result.quit);
}
