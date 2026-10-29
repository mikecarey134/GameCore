#include "doctest.h"
#include "server/ServerConfig.h"
#include "server/ServerConsole.h"
#include "net/EnetTransport.h"
#include "net/LoopbackTransport.h"
#include <chrono>
#include <memory>
#include <thread>

using namespace protocol;

namespace
{
	struct Guest
	{
		Guest(LoopbackNetwork& network, const std::string& address) : transport(network) { transport.setAddress(address); }

		bool join(GameServer& server, const std::string& name)
		{
			if (!transport.connect("server", GameServer::DEFAULT_PORT))
				return false;
			NetEvent event;
			while (transport.poll(event))
				if (event.kind == NetEvent::Connected)
					serverPeer = event.peer;
			transport.send(serverPeer, encode(HELLO, Hello{ PROTOCOL_VERSION, name, 0, {} }), Reliable);
			server.tick();
			return drain();
		}

		bool drain()
		{
			bool welcomed = false;
			NetEvent event;
			while (transport.poll(event))
			{
				if (event.kind == NetEvent::Disconnected)
					dropped = true;
				std::vector<Frame> frames;
				if (event.kind == NetEvent::Received && unframe(event.data, frames))
					for (size_t i = 0; i < frames.size(); ++i)
					{
						ByteReader in(frames[i].payload, frames[i].size);
						Kick kick;
						if (frames[i].type == KICK && read(in, kick))
							kickReason = kick.reason;
						welcomed = welcomed || frames[i].type == WELCOME;
					}
			}
			return welcomed;
		}

		LoopbackTransport transport;
		PeerId serverPeer = NO_PEER;
		bool dropped = false;
		std::string kickReason;
	};

	struct SlowTransport : LoopbackTransport
	{
		explicit SlowTransport(LoopbackNetwork& network) : LoopbackTransport(network) {}
		PeerStats stats(PeerId peer) const override
		{
			PeerStats stats = LoopbackTransport::stats(peer);
			stats.roundTripMs = 40 + peer;
			stats.packetLoss = 0.25f;
			return stats;
		}
	};

	struct Operator
	{
		Operator() : serverTransport(network), server(serverTransport, [] { return 1000u; }, [](const std::string&) {})
		{
			REQUIRE(server.start(GameServer::DEFAULT_PORT));
		}

		Guest& guest(const std::string& address)
		{
			guests.emplace_back(new Guest(network, address));
			return *guests.back();
		}

		std::vector<std::string> run(const std::string& line) { return runConsoleCommand(line, server).output; }

		LoopbackNetwork network;
		SlowTransport serverTransport;
		GameServer server;
		std::vector<std::unique_ptr<Guest> > guests;
	};
}

TEST_CASE("a ban pattern matches whole addresses, with * standing for any run of characters")
{
	CHECK(addressMatches("192.168.1.7", "192.168.1.7"));
	CHECK_FALSE(addressMatches("192.168.1.7", "192.168.1.70"));
	CHECK_FALSE(addressMatches("192.168.1.7", "192.168.1.70x"));
	CHECK(addressMatches("192.168.*", "192.168.1.7"));
	CHECK(addressMatches("192.168.*", "192.168."));
	CHECK_FALSE(addressMatches("192.168.*", "10.192.168.1"));
	CHECK(addressMatches("*.7", "192.168.1.7"));
	CHECK_FALSE(addressMatches("*.7", "192.168.1.70"));
	CHECK(addressMatches("10.*.*.1", "10.0.5.1"));
	CHECK_FALSE(addressMatches("10.*.*.1", "10.0.5.2"));
	CHECK(addressMatches("*", "203.0.113.9"));
}

TEST_CASE_FIXTURE(Operator, "a banned address is told why and dropped on connect, while others still join")
{
	CHECK(run("ban 192.168.*") == std::vector<std::string>{ "Banned 192.168.*" });
	Guest& banned = guest("192.168.1.7");
	CHECK_FALSE(banned.join(server, "rook"));
	CHECK(banned.dropped);
	CHECK(banned.kickReason == "You are banned from this server");
	CHECK(server.playerList().empty());

	Guest& welcome = guest("10.0.0.4");
	CHECK(welcome.join(server, "dana"));
	CHECK_FALSE(welcome.dropped);
	CHECK(server.playerList() == std::vector<std::string>{ "1 dana" });
}

TEST_CASE_FIXTURE(Operator, "unban lets the address back in and banlist shows what is banned")
{
	CHECK(run("banlist") == std::vector<std::string>{ "No bans" });
	run("ban 192.168.*");
	run("ban 10.0.0.9");
	CHECK(run("ban 10.0.0.9") == std::vector<std::string>{ "10.0.0.9 is already banned" });
	CHECK(run("banlist") == std::vector<std::string>{ "192.168.*", "10.0.0.9" });
	CHECK_FALSE(guest("192.168.1.7").join(server, "rook"));

	CHECK(run("unban 192.168.*") == std::vector<std::string>{ "Unbanned 192.168.*" });
	CHECK(run("unban 192.168.*") == std::vector<std::string>{ "No ban for 192.168.*" });
	CHECK(run("banlist") == std::vector<std::string>{ "10.0.0.9" });
	Guest& back = guest("192.168.1.7");
	CHECK(back.join(server, "rook"));
	CHECK_FALSE(back.dropped);
	CHECK(run("ban").size() == 1);
	CHECK(run("ban")[0].find("ban <address>") != std::string::npos);
	CHECK(run("unban")[0].find("unban <address>") != std::string::npos);
}

TEST_CASE_FIXTURE(Operator, "ping and stats list every player with their address, round trip and traffic")
{
	CHECK(run("ping") == std::vector<std::string>{ "No players" });
	CHECK(run("stats") == std::vector<std::string>{ "No players" });
	REQUIRE(guest("10.0.0.4").join(server, "dana"));
	REQUIRE(guest("10.0.0.5").join(server, "rook"));
	CHECK(run("ping") == std::vector<std::string>{ "1 dana 10.0.0.4 41 ms", "2 rook 10.0.0.5 42 ms" });
	PeerStats dana = serverTransport.stats(1);
	REQUIRE(dana.bytesSent > 0);
	REQUIRE(dana.bytesReceived > 0);
	REQUIRE(dana.bytesSent != dana.bytesReceived);
	std::vector<std::string> stats = run("stats");
	REQUIRE(stats.size() == 2);
	CHECK(stats[0] == "1 dana 10.0.0.4: sent " + std::to_string(dana.bytesSent) + " bytes, received " + std::to_string(dana.bytesReceived) +
		" bytes, loss 25%");
	CHECK(stats[1].find("2 rook 10.0.0.5: sent ") == 0);
}

TEST_CASE("the loopback transport counts the bytes each link sends and receives")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	client.setAddress("10.1.2.3");
	REQUIRE(server.listen(4000, 4));
	REQUIRE(client.connect("server", 4000));
	NetEvent event;
	PeerId clientPeer = NO_PEER, serverPeer = NO_PEER;
	while (server.poll(event))
		clientPeer = event.peer;
	while (client.poll(event))
		serverPeer = event.peer;
	CHECK(server.address(clientPeer) == "10.1.2.3");
	CHECK(client.address(serverPeer) == "127.0.0.1");
	client.send(serverPeer, std::vector<uint8_t>(7, 1), Reliable);
	server.send(clientPeer, std::vector<uint8_t>(3, 1), Unreliable);
	CHECK(server.stats(clientPeer).bytesReceived == 7);
	CHECK(server.stats(clientPeer).bytesSent == 3);
	CHECK(client.stats(serverPeer).bytesSent == 7);
	CHECK(client.stats(serverPeer).bytesReceived == 3);
	CHECK(server.address(99) == "");
	CHECK(server.stats(99).bytesSent == 0);
}

TEST_CASE("over ENet the server sees the client's address, its round trip and the bytes both ways")
{
	EnetTransport server, client;
	REQUIRE(server.listen(4311, 4));
	REQUIRE(client.connect("127.0.0.1", 4311));
	PeerId clientPeer = NO_PEER, serverPeer = NO_PEER;
	NetEvent event;
	for (int i = 0; i < 200 && (clientPeer == NO_PEER || serverPeer == NO_PEER); ++i)
	{
		while (server.poll(event))
			if (event.kind == NetEvent::Connected)
				clientPeer = event.peer;
		while (client.poll(event))
			if (event.kind == NetEvent::Connected)
				serverPeer = event.peer;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	REQUIRE(clientPeer != NO_PEER);
	REQUIRE(serverPeer != NO_PEER);
	CHECK(server.address(clientPeer) == "127.0.0.1");
	client.send(serverPeer, std::vector<uint8_t>(50, 2), Reliable);
	bool got = false;
	for (int i = 0; i < 200 && !got; ++i)
	{
		client.poll(event);
		while (server.poll(event))
			got = got || event.kind == NetEvent::Received;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	REQUIRE(got);
	server.send(clientPeer, std::vector<uint8_t>(20, 2), Reliable);
	server.poll(event);
	PeerStats seen = server.stats(clientPeer);
	CHECK(seen.bytesReceived == 50);
	CHECK(seen.bytesSent == 20);
	CHECK(seen.packetLoss == 0.0f);
	CHECK(seen.roundTripMs < 1000);
	CHECK(client.stats(serverPeer).bytesSent == 50);
}

TEST_CASE("the config file sets port and max players, one key and value per line, # for comments")
{
	ServerSettings settings;
	CHECK(settings.port == 1080);
	CHECK(settings.maxPlayers == 8);
	CHECK(settings.configFile == "server.cfg");
	std::vector<std::string> problems = applyServerConfig("# Mike's server settings\nport = 4242\n\n  # slots\nmax_players = 12\n", settings);
	CHECK(problems.empty());
	CHECK(settings.port == 4242);
	CHECK(settings.maxPlayers == 12);
}

TEST_CASE("bad lines in the config file are reported and leave the defaults in place")
{
	ServerSettings settings;
	std::vector<std::string> problems = applyServerConfig("port = banana\nmax_players = 0\ncolour = blue\nport\nport = 70000\n", settings);
	CHECK(problems.size() == 5);
	CHECK(settings.port == 1080);
	CHECK(settings.maxPlayers == 8);
}

TEST_CASE("a missing config file keeps the defaults instead of stopping the server")
{
	ServerSettings settings;
	std::vector<std::string> problems;
	CHECK_FALSE(loadServerConfig("no/such/server.cfg", settings, problems));
	CHECK(problems.empty());
	CHECK(settings.port == 1080);
	CHECK(settings.maxPlayers == 8);
}

TEST_CASE("command line flags override the config file, whichever is read first")
{
	ServerSettings settings;
	std::string error;
	REQUIRE(parseServerArguments({ "--port", "5000", "--config", "other.cfg", "--smoke" }, settings, error));
	CHECK(settings.configFile == "other.cfg");
	CHECK(settings.smoke);
	CHECK(applyServerConfig("port = 4242\nmax_players = 3\n", settings).empty());
	CHECK(settings.port == 5000);
	CHECK(settings.maxPlayers == 3);

	ServerSettings both;
	REQUIRE(parseServerArguments({ "--max-players", "2" }, both, error));
	applyServerConfig("max_players = 6\nport = 4242\n", both);
	CHECK(both.maxPlayers == 2);
	CHECK(both.port == 4242);

	ServerSettings bad;
	CHECK_FALSE(parseServerArguments({ "--port" }, bad, error));
	CHECK(error.find("Usage") != std::string::npos);
	CHECK_FALSE(parseServerArguments({ "--port", "0" }, bad, error));
	CHECK_FALSE(parseServerArguments({ "--max-players", "x" }, bad, error));
	CHECK_FALSE(parseServerArguments({ "--colour" }, bad, error));
}

TEST_CASE("the server holds no more players than it was started for")
{
	LoopbackNetwork network;
	LoopbackTransport transport(network);
	GameServer small(transport, [] { return 1000u; }, [](const std::string&) {});
	REQUIRE(small.start(GameServer::DEFAULT_PORT, 1));
	Guest first(network, "10.0.0.1"), second(network, "10.0.0.2");
	CHECK(first.join(small, "a"));
	CHECK_FALSE(second.transport.connect("server", GameServer::DEFAULT_PORT));
}

TEST_CASE("over ENet a packet sent just before a disconnect still reaches the client, so a kick or ban carries its reason")
{
	EnetTransport server, client;
	REQUIRE(server.listen(4312, 4));
	REQUIRE(client.connect("127.0.0.1", 4312));
	PeerId clientPeer = NO_PEER;
	NetEvent event;
	for (int i = 0; i < 200 && clientPeer == NO_PEER; ++i)
	{
		while (server.poll(event))
			if (event.kind == NetEvent::Connected)
				clientPeer = event.peer;
		client.poll(event);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	REQUIRE(clientPeer != NO_PEER);
	server.send(clientPeer, std::vector<uint8_t>{ 4, 2 }, Reliable);
	server.disconnect(clientPeer);
	std::vector<NetEvent::Kind> seen;
	for (int i = 0; i < 400 && (seen.empty() || seen.back() != NetEvent::Disconnected); ++i)
	{
		server.poll(event);
		while (client.poll(event))
			if (event.kind != NetEvent::Connected)
				seen.push_back(event.kind);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	CHECK(seen == std::vector<NetEvent::Kind>{ NetEvent::Received, NetEvent::Disconnected });
}
