#include "doctest.h"
#include "net/EnetTransport.h"
#include "net/MessageFrame.h"
#include "net/MessageRouter.h"
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

namespace
{
	struct Endpoint
	{
		explicit Endpoint(uint32_t timeoutMs = EnetTransport::DEFAULT_TIMEOUT_MS) : transport(timeoutMs) {}

		int count(NetEvent::Kind kind) const
		{
			int total = 0;
			for (size_t i = 0; i < events.size(); ++i)
				if (events[i].kind == kind)
					++total;
			return total;
		}

		const NetEvent* first(NetEvent::Kind kind) const
		{
			for (size_t i = 0; i < events.size(); ++i)
				if (events[i].kind == kind)
					return &events[i];
			return nullptr;
		}

		EnetTransport transport;
		std::vector<NetEvent> events;
	};

	bool pumpUntil(std::vector<Endpoint*> endpoints, std::function<bool()> done, int milliseconds = 1000)
	{
		std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
		while (true)
		{
			for (size_t i = 0; i < endpoints.size(); ++i)
			{
				NetEvent event;
				while (endpoints[i]->transport.poll(event))
					endpoints[i]->events.push_back(event);
			}
			if (done())
				return true;
			if (std::chrono::steady_clock::now() > deadline)
				return false;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void pumpFor(std::vector<Endpoint*> endpoints, int milliseconds)
	{
		pumpUntil(endpoints, [] { return false; }, milliseconds);
	}

	uint16_t listenOnFreePort(EnetTransport& transport, int maxPeers)
	{
		static uint16_t next = (uint16_t)(49152 + std::chrono::steady_clock::now().time_since_epoch().count() % 10000);
		for (int attempt = 0; attempt < 50; ++attempt)
		{
			uint16_t port = next++;
			if (next < 49152)
				next = 49152;
			if (transport.listen(port, maxPeers))
				return port;
		}
		return 0;
	}

	PeerId connectTo(Endpoint& client, Endpoint& server, uint16_t port)
	{
		int before = server.count(NetEvent::Connected);
		REQUIRE(client.transport.connect("127.0.0.1", port));
		REQUIRE(pumpUntil({ &client, &server }, [&] {
			return client.count(NetEvent::Connected) >= 1 && server.count(NetEvent::Connected) > before;
		}));
		return server.events.back().kind == NetEvent::Connected ? server.events.back().peer : NO_PEER;
	}

	PeerId lastConnected(const Endpoint& endpoint)
	{
		for (size_t i = endpoint.events.size(); i-- > 0;)
			if (endpoint.events[i].kind == NetEvent::Connected)
				return endpoint.events[i].peer;
		return NO_PEER;
	}

	const std::vector<uint8_t> hello = { 1, 2, 3 };
}

TEST_CASE("enet: connecting raises Connected on both sides, each with a real id")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	REQUIRE(client.transport.connect("127.0.0.1", port));
	CHECK(pumpUntil({ &client, &server }, [&] {
		return client.count(NetEvent::Connected) == 1 && server.count(NetEvent::Connected) == 1;
	}));
	CHECK(lastConnected(client) != NO_PEER);
	CHECK(lastConnected(server) != NO_PEER);
}

TEST_CASE("enet: connecting to a port nobody listens on ends in Disconnected")
{
	Endpoint unused, client(300);
	uint16_t port = listenOnFreePort(unused.transport, 1);
	REQUIRE(port != 0);
	unused.transport.shutdown();
	REQUIRE(client.transport.connect("127.0.0.1", port));
	CHECK(pumpUntil({ &client }, [&] { return client.count(NetEvent::Disconnected) == 1; }, 3000));
	CHECK(client.count(NetEvent::Connected) == 0);
}

TEST_CASE("enet: a second listener cannot take a port in use")
{
	Endpoint first, second;
	uint16_t port = listenOnFreePort(first.transport, 4);
	REQUIRE(port != 0);
	CHECK_FALSE(second.transport.listen(port, 4));
}

TEST_CASE("enet: a transport listens or connects once")
{
	Endpoint server, other;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	CHECK_FALSE(server.transport.listen((uint16_t)(port + 1), 4));
	CHECK_FALSE(server.transport.connect("127.0.0.1", port));
}

TEST_CASE("enet: reliable bytes arrive in order, tagged with the sender as the receiver knows it")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	PeerId clientOnServer = connectTo(client, server, port);
	PeerId serverOnClient = lastConnected(client);

	client.transport.send(serverOnClient, hello, Reliable);
	client.transport.send(serverOnClient, { 4 }, Reliable);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return server.count(NetEvent::Received) == 2; }));
	std::vector<NetEvent> received;
	for (size_t i = 0; i < server.events.size(); ++i)
		if (server.events[i].kind == NetEvent::Received)
			received.push_back(server.events[i]);
	CHECK(received[0].peer == clientOnServer);
	CHECK(received[0].data == hello);
	CHECK(received[1].data == std::vector<uint8_t>{ 4 });

	server.transport.send(clientOnServer, { 9 }, Reliable);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return client.count(NetEvent::Received) == 1; }));
	CHECK(client.first(NetEvent::Received)->peer == serverOnClient);
	CHECK(client.first(NetEvent::Received)->data == std::vector<uint8_t>{ 9 });
}

TEST_CASE("enet: an unreliable message arrives over localhost")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	connectTo(client, server, port);
	client.transport.send(lastConnected(client), { 7, 7 }, Unreliable);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return server.count(NetEvent::Received) == 1; }));
	CHECK(server.first(NetEvent::Received)->data == std::vector<uint8_t>{ 7, 7 });
}

TEST_CASE("enet: a reliable message bigger than one datagram arrives whole")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	connectTo(client, server, port);
	std::vector<uint8_t> big(5000);
	for (size_t i = 0; i < big.size(); ++i)
		big[i] = (uint8_t)(i * 7);
	client.transport.send(lastConnected(client), big, Reliable);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return server.count(NetEvent::Received) == 1; }));
	CHECK(server.first(NetEvent::Received)->data == big);
}

TEST_CASE("enet: with several clients, every message is tagged with the right sender")
{
	Endpoint server, a, b;
	uint16_t port = listenOnFreePort(server.transport, 8);
	REQUIRE(port != 0);
	PeerId aOnServer = connectTo(a, server, port);
	PeerId bOnServer = connectTo(b, server, port);
	REQUIRE(aOnServer != bOnServer);

	b.transport.send(lastConnected(b), hello, Reliable);
	REQUIRE(pumpUntil({ &server, &a, &b }, [&] { return server.count(NetEvent::Received) == 1; }));
	CHECK(server.first(NetEvent::Received)->peer == bOnServer);

	server.transport.send(bOnServer, hello, Reliable);
	REQUIRE(pumpUntil({ &server, &a, &b }, [&] { return b.count(NetEvent::Received) == 1; }));
	CHECK(b.first(NetEvent::Received)->peer == lastConnected(b));
	pumpFor({ &server, &a, &b }, 50);
	CHECK(a.count(NetEvent::Received) == 0);
}

TEST_CASE("enet: broadcast reaches every peer except the one excluded")
{
	Endpoint server, a, b, c;
	uint16_t port = listenOnFreePort(server.transport, 8);
	REQUIRE(port != 0);
	PeerId aOnServer = connectTo(a, server, port);
	connectTo(b, server, port);
	connectTo(c, server, port);

	server.transport.broadcast(hello, Reliable, aOnServer);
	REQUIRE(pumpUntil({ &server, &a, &b, &c }, [&] {
		return b.count(NetEvent::Received) == 1 && c.count(NetEvent::Received) == 1;
	}));
	pumpFor({ &server, &a, &b, &c }, 50);
	CHECK(a.count(NetEvent::Received) == 0);
	CHECK(b.first(NetEvent::Received)->data == hello);

	server.transport.broadcast({ 5 }, Reliable, NO_PEER);
	CHECK(pumpUntil({ &server, &a, &b, &c }, [&] {
		return a.count(NetEvent::Received) == 1 && b.count(NetEvent::Received) == 2 && c.count(NetEvent::Received) == 2;
	}));
}

TEST_CASE("enet: disconnecting raises Disconnected on both sides and stops delivery")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	PeerId clientOnServer = connectTo(client, server, port);
	PeerId serverOnClient = lastConnected(client);

	client.transport.disconnect(serverOnClient);
	REQUIRE(pumpUntil({ &client, &server }, [&] {
		return client.count(NetEvent::Disconnected) == 1 && server.count(NetEvent::Disconnected) == 1;
	}));
	CHECK(server.first(NetEvent::Disconnected)->peer == clientOnServer);
	CHECK(client.first(NetEvent::Disconnected)->peer == serverOnClient);

	server.transport.send(clientOnServer, hello, Reliable);
	client.transport.send(serverOnClient, hello, Reliable);
	pumpFor({ &client, &server }, 50);
	CHECK(client.count(NetEvent::Received) == 0);
	CHECK(server.count(NetEvent::Received) == 0);
}

TEST_CASE("enet: a full server refuses another client")
{
	Endpoint server, a, late(300);
	uint16_t port = listenOnFreePort(server.transport, 1);
	REQUIRE(port != 0);
	connectTo(a, server, port);
	REQUIRE(late.transport.connect("127.0.0.1", port));
	CHECK(pumpUntil({ &server, &a, &late }, [&] { return late.count(NetEvent::Disconnected) == 1; }, 3000));
	CHECK(late.count(NetEvent::Connected) == 0);
	CHECK(server.count(NetEvent::Connected) == 1);
}

TEST_CASE("enet: ids are not reused after a client leaves")
{
	Endpoint server, a, b;
	uint16_t port = listenOnFreePort(server.transport, 1);
	REQUIRE(port != 0);
	PeerId aOnServer = connectTo(a, server, port);
	a.transport.disconnect(lastConnected(a));
	REQUIRE(pumpUntil({ &server, &a }, [&] { return server.count(NetEvent::Disconnected) == 1; }));
	PeerId bOnServer = connectTo(b, server, port);
	CHECK(bOnServer != NO_PEER);
	CHECK(bOnServer != aOnServer);

	server.transport.send(aOnServer, hello, Reliable);
	pumpFor({ &server, &b }, 50);
	CHECK(b.count(NetEvent::Received) == 0);
}

TEST_CASE("enet: a client's broadcast reaches the server once")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	connectTo(client, server, port);
	client.transport.broadcast(hello, Reliable, NO_PEER);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return server.count(NetEvent::Received) >= 1; }));
	pumpFor({ &client, &server }, 50);
	CHECK(server.count(NetEvent::Received) == 1);
}

TEST_CASE("enet: a message sent before the connection completes is dropped")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	REQUIRE(client.transport.connect("127.0.0.1", port));
	client.transport.broadcast(hello, Reliable, NO_PEER);
	REQUIRE(pumpUntil({ &client, &server }, [&] {
		return client.count(NetEvent::Connected) == 1 && server.count(NetEvent::Connected) == 1;
	}));
	pumpFor({ &client, &server }, 50);
	CHECK(server.count(NetEvent::Received) == 0);
}

TEST_CASE("enet: connecting to a host name that cannot resolve fails at once")
{
	Endpoint client;
	CHECK_FALSE(client.transport.connect("nonexistent.invalid", 50000));
	NetEvent event;
	CHECK_FALSE(client.transport.poll(event));
	Endpoint again;
	REQUIRE(listenOnFreePort(again.transport, 1) != 0);
}

TEST_CASE("enet: shutting down disconnects every peer and frees the port")
{
	Endpoint server, a, b, replacement;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	connectTo(a, server, port);
	connectTo(b, server, port);

	server.transport.shutdown();
	CHECK(pumpUntil({ &a, &b }, [&] {
		return a.count(NetEvent::Disconnected) == 1 && b.count(NetEvent::Disconnected) == 1;
	}));
	CHECK(replacement.transport.listen(port, 4));
}

TEST_CASE("enet: a transport that goes away disconnects its peers")
{
	Endpoint server;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	{
		Endpoint client;
		connectTo(client, server, port);
	}
	CHECK(pumpUntil({ &server }, [&] { return server.count(NetEvent::Disconnected) == 1; }));
}

TEST_CASE("enet: sending to a peer that does not exist does nothing")
{
	Endpoint server;
	REQUIRE(listenOnFreePort(server.transport, 4) != 0);
	server.transport.send(42, hello, Reliable);
	server.transport.disconnect(42);
	server.transport.broadcast(hello, Reliable, NO_PEER);
	pumpFor({ &server }, 20);
	CHECK(server.events.empty());

	Endpoint idle;
	idle.transport.send(1, hello, Reliable);
	NetEvent event;
	CHECK_FALSE(idle.transport.poll(event));
}

TEST_CASE("enet: a framed message crosses the transport and reaches its handler with the sender's id")
{
	Endpoint server, client;
	uint16_t port = listenOnFreePort(server.transport, 4);
	REQUIRE(port != 0);
	PeerId clientOnServer = connectTo(client, server, port);

	ByteWriter chat;
	chat.writeString("hi there", 200);
	client.transport.send(lastConnected(client), frameMessage(17, chat.data()), Reliable);
	REQUIRE(pumpUntil({ &client, &server }, [&] { return server.count(NetEvent::Received) == 1; }));

	std::vector<std::string> log;
	MessageRouter router([&](const std::string& line) { log.push_back(line); });
	PeerId from = NO_PEER;
	std::string text;
	router.on(17, [&](PeerId peer, ByteReader& reader) {
		from = peer;
		text = reader.readString(200);
	});
	const NetEvent* received = server.first(NetEvent::Received);
	router.route(received->peer, received->data);
	CHECK(from == clientOnServer);
	CHECK(text == "hi there");
	CHECK(log.empty());
}
