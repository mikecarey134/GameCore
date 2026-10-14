#include "doctest.h"
#include "net/LoopbackTransport.h"
#include "net/MessageFrame.h"
#include "net/MessageRouter.h"

namespace
{
	std::vector<NetEvent> drain(Transport& transport)
	{
		std::vector<NetEvent> events;
		NetEvent event;
		while (transport.poll(event))
			events.push_back(event);
		return events;
	}

	PeerId connectedPeer(Transport& transport)
	{
		std::vector<NetEvent> events = drain(transport);
		REQUIRE(events.size() == 1);
		REQUIRE(events[0].kind == NetEvent::Connected);
		return events[0].peer;
	}

	const std::vector<uint8_t> hello = { 1, 2, 3 };
}

TEST_CASE("connecting raises Connected on both sides, each with its own id for the other")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	REQUIRE(server.listen(7000, 4));
	REQUIRE(client.connect("localhost", 7000));
	PeerId clientOnServer = connectedPeer(server);
	PeerId serverOnClient = connectedPeer(client);
	CHECK(clientOnServer != NO_PEER);
	CHECK(serverOnClient != NO_PEER);
}

TEST_CASE("connecting to a port nobody listens on fails quietly")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	REQUIRE(server.listen(7000, 4));
	CHECK_FALSE(client.connect("localhost", 7001));
	CHECK(drain(client).empty());
	CHECK(drain(server).empty());
}

TEST_CASE("a second listener cannot take a port in use")
{
	LoopbackNetwork network;
	LoopbackTransport first(network), second(network);
	CHECK(first.listen(7000, 4));
	CHECK_FALSE(second.listen(7000, 4));
	CHECK(second.listen(7001, 4));
}

TEST_CASE("sent bytes arrive in order, tagged with the sender as the receiver knows it")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	PeerId clientOnServer = connectedPeer(server);
	PeerId serverOnClient = connectedPeer(client);

	client.send(serverOnClient, hello, Reliable);
	client.send(serverOnClient, { 4 }, Unreliable);
	std::vector<NetEvent> atServer = drain(server);
	REQUIRE(atServer.size() == 2);
	CHECK(atServer[0].kind == NetEvent::Received);
	CHECK(atServer[0].peer == clientOnServer);
	CHECK(atServer[0].data == hello);
	CHECK(atServer[1].data == std::vector<uint8_t>{ 4 });

	server.send(clientOnServer, { 9 }, Reliable);
	std::vector<NetEvent> atClient = drain(client);
	REQUIRE(atClient.size() == 1);
	CHECK(atClient[0].peer == serverOnClient);
	CHECK(atClient[0].data == std::vector<uint8_t>{ 9 });
	CHECK(drain(server).empty());
}

TEST_CASE("with several clients, every message is tagged with the id the receiver uses for its sender")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network);
	server.listen(7000, 8);
	a.connect("localhost", 7000);
	PeerId aOnServer = connectedPeer(server);
	b.connect("localhost", 7000);
	PeerId bOnServer = connectedPeer(server);
	PeerId serverOnB = connectedPeer(b);
	drain(a);
	REQUIRE(aOnServer != bOnServer);

	b.send(serverOnB, hello, Reliable);
	std::vector<NetEvent> atServer = drain(server);
	REQUIRE(atServer.size() == 1);
	CHECK(atServer[0].peer == bOnServer);

	server.send(bOnServer, hello, Reliable);
	std::vector<NetEvent> atB = drain(b);
	REQUIRE(atB.size() == 1);
	CHECK(atB[0].peer == serverOnB);
	CHECK(drain(a).empty());
}

TEST_CASE("each client gets a different id on the server")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network), c(network);
	server.listen(7000, 8);
	a.connect("localhost", 7000);
	b.connect("localhost", 7000);
	c.connect("localhost", 7000);
	std::vector<NetEvent> events = drain(server);
	REQUIRE(events.size() == 3);
	CHECK(events[0].peer != events[1].peer);
	CHECK(events[1].peer != events[2].peer);
	CHECK(events[0].peer != events[2].peer);
}

TEST_CASE("broadcast reaches every peer except the one excluded")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network), c(network);
	server.listen(7000, 8);
	a.connect("localhost", 7000);
	PeerId aOnServer = connectedPeer(server);
	b.connect("localhost", 7000);
	c.connect("localhost", 7000);
	drain(server);
	drain(a);
	drain(b);
	drain(c);

	server.broadcast(hello, Reliable, aOnServer);
	CHECK(drain(a).empty());
	std::vector<NetEvent> atB = drain(b), atC = drain(c);
	REQUIRE(atB.size() == 1);
	REQUIRE(atC.size() == 1);
	CHECK(atB[0].data == hello);
	CHECK(atC[0].data == hello);

	server.broadcast({ 5 }, Unreliable, NO_PEER);
	CHECK(drain(a).size() == 1);
	CHECK(drain(b).size() == 1);
	CHECK(drain(c).size() == 1);
}

TEST_CASE("disconnecting raises Disconnected on both sides and stops delivery")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	PeerId clientOnServer = connectedPeer(server);
	PeerId serverOnClient = connectedPeer(client);

	client.disconnect(serverOnClient);
	std::vector<NetEvent> atServer = drain(server), atClient = drain(client);
	REQUIRE(atServer.size() == 1);
	CHECK(atServer[0].kind == NetEvent::Disconnected);
	CHECK(atServer[0].peer == clientOnServer);
	REQUIRE(atClient.size() == 1);
	CHECK(atClient[0].kind == NetEvent::Disconnected);
	CHECK(atClient[0].peer == serverOnClient);

	server.send(clientOnServer, hello, Reliable);
	client.send(serverOnClient, hello, Reliable);
	CHECK(drain(client).empty());
	CHECK(drain(server).empty());
}

TEST_CASE("a full server refuses another client")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network), c(network);
	server.listen(7000, 2);
	CHECK(a.connect("localhost", 7000));
	CHECK(b.connect("localhost", 7000));
	CHECK_FALSE(c.connect("localhost", 7000));
	CHECK(drain(server).size() == 2);
	CHECK(drain(c).empty());
}

TEST_CASE("a slot frees up when a client leaves")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network);
	server.listen(7000, 1);
	a.connect("localhost", 7000);
	PeerId serverOnA = connectedPeer(a);
	a.disconnect(serverOnA);
	CHECK(b.connect("localhost", 7000));
}

TEST_CASE("shutting down disconnects every peer and frees the port")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), a(network), b(network), replacement(network);
	server.listen(7000, 4);
	a.connect("localhost", 7000);
	b.connect("localhost", 7000);
	drain(a);
	drain(b);
	drain(server);

	server.shutdown();
	std::vector<NetEvent> atA = drain(a), atB = drain(b);
	REQUIRE(atA.size() == 1);
	CHECK(atA[0].kind == NetEvent::Disconnected);
	REQUIRE(atB.size() == 1);
	CHECK(atB[0].kind == NetEvent::Disconnected);
	CHECK(replacement.listen(7000, 4));
}

TEST_CASE("a transport that shuts down keeps no old events, so it can be reused like a fresh one")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	drain(server);

	client.shutdown();
	server.shutdown();
	CHECK(drain(client).empty());
	CHECK(drain(server).empty());

	REQUIRE(server.listen(7000, 4));
	REQUIRE(client.connect("localhost", 7000));
	connectedPeer(client);
	connectedPeer(server);
}

TEST_CASE("a transport that goes away disconnects its peers")
{
	LoopbackNetwork network;
	LoopbackTransport server(network);
	server.listen(7000, 4);
	{
		LoopbackTransport client(network);
		client.connect("localhost", 7000);
		drain(server);
	}
	std::vector<NetEvent> events = drain(server);
	REQUIRE(events.size() == 1);
	CHECK(events[0].kind == NetEvent::Disconnected);
}

TEST_CASE("sending to a peer that does not exist does nothing")
{
	LoopbackNetwork network;
	LoopbackTransport server(network);
	server.listen(7000, 4);
	server.send(42, hello, Reliable);
	server.disconnect(42);
	CHECK(drain(server).empty());
}

TEST_CASE("a framed message crosses the loopback and reaches its handler with the sender's id")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	PeerId clientOnServer = connectedPeer(server);
	PeerId serverOnClient = connectedPeer(client);

	ByteWriter chat;
	chat.writeString("hi there", 200);
	client.send(serverOnClient, frameMessage(17, chat.data()), Reliable);

	std::vector<std::string> log;
	MessageRouter router([&](const std::string& line) { log.push_back(line); });
	PeerId from = NO_PEER;
	std::string text;
	router.on(17, [&](PeerId peer, ByteReader& reader) {
		from = peer;
		text = reader.readString(200);
	});
	NetEvent event;
	while (server.poll(event))
		if (event.kind == NetEvent::Received)
			router.route(event.peer, event.data);
	CHECK(from == clientOnServer);
	CHECK(text == "hi there");
	CHECK(log.empty());
}

TEST_CASE("a lossy link drops unreliable packets and never reliable ones")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	drain(server);
	PeerId serverOnClient = connectedPeer(client);
	client.setUnreliableLoss(1.0f, 0.0f, 7);
	for (uint8_t i = 0; i < 10; ++i)
	{
		client.send(serverOnClient, { i }, Unreliable);
		client.send(serverOnClient, { (uint8_t)(100 + i) }, Reliable);
	}
	std::vector<NetEvent> events = drain(server);
	REQUIRE(events.size() == 10);
	for (size_t i = 0; i < events.size(); ++i)
		CHECK(events[i].data[0] == 100 + i);
}

TEST_CASE("a lossy link with some loss drops some unreliable packets but not all")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	drain(server);
	PeerId serverOnClient = connectedPeer(client);
	client.setUnreliableLoss(0.5f, 0.0f, 7);
	for (int i = 0; i < 200; ++i)
		client.send(serverOnClient, { 1 }, Unreliable);
	size_t arrived = drain(server).size();
	CHECK(arrived > 50);
	CHECK(arrived < 150);
}

TEST_CASE("a reordering link delivers unreliable packets out of order")
{
	LoopbackNetwork network;
	LoopbackTransport server(network), client(network);
	server.listen(7000, 4);
	client.connect("localhost", 7000);
	drain(server);
	PeerId serverOnClient = connectedPeer(client);
	client.setUnreliableLoss(0.0f, 1.0f, 7);
	for (uint8_t i = 1; i <= 4; ++i)
		client.send(serverOnClient, { i }, Unreliable);
	client.send(serverOnClient, { 9 }, Reliable);
	std::vector<NetEvent> events = drain(server);
	REQUIRE(events.size() == 5);
	CHECK(events[0].data[0] == 2);
	CHECK(events[1].data[0] == 1);
	CHECK(events[2].data[0] == 4);
	CHECK(events[3].data[0] == 3);
	CHECK(events[4].data[0] == 9);
}
