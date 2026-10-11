#include "doctest.h"
#include "server/GameServer.h"
#include "net/EnetTransport.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

using namespace protocol;

namespace
{
	struct RealClient
	{
		RealClient() : server(NO_PEER), connected(false), disconnected(false) {}

		void pump()
		{
			NetEvent event;
			while (transport.poll(event))
			{
				if (event.kind == NetEvent::Connected)
				{
					server = event.peer;
					connected = true;
				}
				else if (event.kind == NetEvent::Disconnected)
					disconnected = true;
				else
				{
					std::vector<Frame> frames;
					if (unframe(event.data, frames))
						for (size_t i = 0; i < frames.size(); ++i)
							received.push_back(std::make_pair(frames[i].type,
								std::vector<uint8_t>(frames[i].payload, frames[i].payload + frames[i].size)));
				}
			}
		}

		template <class Message>
		bool last(MessageType type, Message& message) const
		{
			for (size_t i = received.size(); i-- > 0;)
				if (received[i].first == type)
				{
					ByteReader in(received[i].second);
					return read(in, message);
				}
			return false;
		}

		bool has(MessageType type) const
		{
			for (size_t i = 0; i < received.size(); ++i)
				if (received[i].first == type)
					return true;
			return false;
		}

		EnetTransport transport;
		PeerId server;
		bool connected;
		bool disconnected;
		std::vector<std::pair<uint8_t, std::vector<uint8_t> > > received;
	};

	bool waitFor(std::vector<RealClient*> clients, std::function<bool()> done)
	{
		std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (std::chrono::steady_clock::now() < deadline)
		{
			for (size_t i = 0; i < clients.size(); ++i)
				clients[i]->pump();
			if (done())
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return false;
	}

	void joinServer(RealClient& client, uint16_t port, const std::string& name)
	{
		REQUIRE(client.transport.connect("127.0.0.1", port));
		REQUIRE(waitFor({ &client }, [&] { return client.connected; }));
		client.transport.send(client.server, encode(HELLO, Hello{ PROTOCOL_VERSION, name, 1 }), Reliable);
		REQUIRE(waitFor({ &client }, [&] { return client.has(WELCOME); }));
	}
}

TEST_CASE("two real clients meet, chat and part through the server over localhost")
{
	EnetTransport serverTransport;
	std::mutex logLock;
	std::vector<std::string> log;
	std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
	GameServer server(serverTransport,
		[started] { return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count(); },
		[&](const std::string& line) { std::lock_guard<std::mutex> guard(logLock); log.push_back(line); });

	uint16_t port = (uint16_t)(50000 + std::chrono::steady_clock::now().time_since_epoch().count() % 10000);
	int attempts = 0;
	while (!server.start(port) && ++attempts < 50)
		++port;
	REQUIRE(attempts < 50);

	std::atomic<bool> running(true);
	std::thread loop([&] {
		while (running)
		{
			server.tick();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		server.stop();
	});

	RealClient a, b;
	joinServer(a, port, "mikey");
	joinServer(b, port, "dan");

	Welcome welcome;
	REQUIRE(a.last(WELCOME, welcome));
	CHECK(welcome.yourId == 1);
	REQUIRE(b.last(WELCOME, welcome));
	CHECK(welcome.yourId == 2);
	REQUIRE(welcome.players.size() == 1);
	CHECK(welcome.players[0].name == "mikey");

	PlayerJoined joined;
	CHECK(waitFor({ &a, &b }, [&] { return a.has(PLAYER_JOINED); }));
	REQUIRE(a.last(PLAYER_JOINED, joined));
	CHECK(joined.player.id == 2);
	CHECK(joined.player.name == "dan");

	a.transport.send(a.server, encode(CHAT, Chat{ "hi from mikey" }), Reliable);
	CHECK(waitFor({ &a, &b }, [&] { return b.has(CHAT); }));
	RelayedChat chat;
	REQUIRE(b.last(CHAT, chat));
	CHECK(chat.id == 1);
	CHECK(chat.text == "hi from mikey");

	b.transport.disconnect(b.server);
	CHECK(waitFor({ &a, &b }, [&] { return a.has(PLAYER_LEFT); }));
	PlayerLeft left;
	REQUIRE(a.last(PLAYER_LEFT, left));
	CHECK(left.id == 2);

	running = false;
	loop.join();
	CHECK(waitFor({ &a }, [&] { return a.disconnected; }));
}
