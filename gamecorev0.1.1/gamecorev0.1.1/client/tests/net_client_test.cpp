#include "doctest.h"
#include "client/NetClient.h"
#include "server/GameServer.h"
#include "net/LoopbackTransport.h"
#include "net/Interpolator.h"
#include <memory>

using namespace protocol;

namespace
{
	struct Seen
	{
		std::vector<std::pair<std::string, NetClient::LineKind> > lines;
		std::vector<uint16_t> joined, left, died, respawned, damagedBy;
		std::vector<NetClient::Remote> moved;
		std::vector<std::pair<float, float> > clocks;

		bool hasLine(const std::string& text) const
		{
			for (size_t i = 0; i < lines.size(); ++i)
				if (lines[i].first.find(text) != std::string::npos)
					return true;
			return false;
		}
	};

	struct TestClient
	{
		TestClient(LoopbackNetwork& network, uint32_t& now) : transport(network), client(transport, [&now] { return now; }, events())
		{
			local = State{ 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0 };
		}

		NetClient::Events events()
		{
			NetClient::Events e;
			e.line = [this](const std::string& text, NetClient::LineKind kind) { seen.lines.push_back(std::make_pair(text, kind)); };
			e.joined = [this](const NetClient::Remote& remote) { seen.joined.push_back(remote.id); };
			e.left = [this](uint16_t id) { seen.left.push_back(id); };
			e.moved = [this](const NetClient::Remote& remote) { seen.moved.push_back(remote); };
			e.died = [this](uint16_t id) { seen.died.push_back(id); };
			e.respawned = [this](const NetClient::Remote& remote) { seen.respawned.push_back(remote.id); };
			e.damaged = [this](uint16_t byId) { seen.damagedBy.push_back(byId); };
			e.clock = [this](float timeOfDay, float timeScale) { seen.clocks.push_back(std::make_pair(timeOfDay, timeScale)); };
			return e;
		}

		LoopbackTransport transport;
		Seen seen;
		NetClient client;
		State local;
	};

	struct ClientFixture
	{
		ClientFixture()
			: now(1000), serverTransport(network),
			  server(serverTransport, [this] { return now; }, [](const std::string&) {})
		{
			REQUIRE(server.start(GameServer::DEFAULT_PORT));
		}

		TestClient& add()
		{
			clients.push_back(std::unique_ptr<TestClient>(new TestClient(network, now)));
			return *clients.back();
		}

		void settle()
		{
			for (int round = 0; round < 3; ++round)
			{
				server.tick();
				for (size_t i = 0; i < clients.size(); ++i)
					clients[i]->client.update(clients[i]->local);
			}
		}

		TestClient& joined(const std::string& name)
		{
			TestClient& client = add();
			client.client.setProfile(std::vector<uint8_t>(name.rbegin(), name.rend()));
			REQUIRE(client.client.connect("localhost", GameServer::DEFAULT_PORT, name, 1));
			settle();
			REQUIRE(client.client.isJoined());
			return client;
		}

		void advance(uint32_t milliseconds)
		{
			now += milliseconds;
			settle();
		}

		uint32_t now;
		LoopbackNetwork network;
		LoopbackTransport serverTransport;
		GameServer server;
		std::vector<std::unique_ptr<TestClient> > clients;
	};
}

TEST_CASE_FIXTURE(ClientFixture, "connecting says hello and the welcome gives the local id")
{
	TestClient& a = add();
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	CHECK(a.client.isOnline());
	CHECK_FALSE(a.client.isJoined());
	settle();
	CHECK(a.client.isJoined());
	CHECK(a.client.localId() == 1);
	CHECK(a.seen.hasLine("Connected"));
	TestClient& b = joined("dan");
	CHECK(b.client.localId() == 2);
}

TEST_CASE_FIXTURE(ClientFixture, "players already in the game and players who join later both appear")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	CHECK(b.seen.joined == std::vector<uint16_t>{ 1 });
	CHECK(a.seen.joined == std::vector<uint16_t>{ 2 });
	REQUIRE(a.client.players().count(2) == 1);
	CHECK(a.client.players().at(2).name == "dan");
	CHECK(a.client.players().at(2).model == 1);
	CHECK(a.client.players().at(2).profile == std::vector<uint8_t>{ 'n', 'a', 'd' });
	REQUIRE(b.client.players().count(1) == 1);
	CHECK(b.client.players().at(1).profile == std::vector<uint8_t>{ 'y', 'e', 'k', 'i', 'm' });
	CHECK(a.seen.hasLine("dan Joined"));
}

TEST_CASE_FIXTURE(ClientFixture, "a relayed state moves the remote player")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.local = State{ 0, { 10.0f, 2.0f, -3.0f }, { 0.0f, 90.0f, 0.0f }, 3 };
	advance(NetClient::STATE_INTERVAL_MS);
	REQUIRE_FALSE(a.seen.moved.empty());
	const NetClient::Remote& remote = a.seen.moved.back();
	CHECK(remote.id == 2);
	CHECK(remote.state.pos[0] == 10.0f);
	CHECK(remote.state.pos[2] == -3.0f);
	CHECK(remote.state.rot[1] == 90.0f);
	CHECK(remote.state.state == 3);
	CHECK(a.client.players().at(2).hasState);
}

TEST_CASE_FIXTURE(ClientFixture, "chat from another player shows as name and text, our own is echoed")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	a.client.sendChat("hello all");
	settle();
	CHECK(b.seen.hasLine("mikey: hello all"));
	CHECK(a.seen.hasLine("mikey: hello all"));
}

TEST_CASE_FIXTURE(ClientFixture, "a valid hit makes the server damage the target")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.local.pos[0] = 5.0f;
	advance(NetClient::STATE_INTERVAL_MS);
	a.client.sendHit(2);
	settle();
	CHECK(b.seen.damagedBy == std::vector<uint16_t>{ 1 });
	CHECK(a.seen.damagedBy.empty());
}

TEST_CASE_FIXTURE(ClientFixture, "a death hides the player and their next alive state shows them again")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.client.sendDied();
	b.local.state = STATE_DEAD;
	advance(NetClient::STATE_INTERVAL_MS);
	CHECK(a.seen.died == std::vector<uint16_t>{ 2 });
	CHECK(a.seen.hasLine("dan Died"));
	CHECK_FALSE(a.client.players().at(2).alive);
	CHECK(a.seen.respawned.empty());

	b.local.state = 0;
	advance(NetClient::STATE_INTERVAL_MS);
	CHECK(a.seen.respawned == std::vector<uint16_t>{ 2 });
	CHECK(a.client.players().at(2).alive);
}

TEST_CASE_FIXTURE(ClientFixture, "a kick shows the reason and takes the client offline")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	REQUIRE(server.kick("dan"));
	settle();
	CHECK_FALSE(b.client.isOnline());
	CHECK_FALSE(b.client.isJoined());
	CHECK(b.seen.hasLine("Kicked by the server"));
	CHECK(b.seen.left == std::vector<uint16_t>{ 1 });
	CHECK(b.client.players().empty());
	CHECK(a.seen.left == std::vector<uint16_t>{ 2 });
}

TEST_CASE_FIXTURE(ClientFixture, "a player who leaves is removed and announced")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.client.shutdown();
	settle();
	CHECK(a.seen.left == std::vector<uint16_t>{ 2 });
	CHECK(a.client.players().count(2) == 0);
	CHECK(a.seen.hasLine("dan Left"));
	CHECK_FALSE(b.client.isOnline());
}

TEST_CASE_FIXTURE(ClientFixture, "the local state goes out twenty times a second, not every frame")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.seen.moved.clear();
	for (int frame = 0; frame < 100; ++frame)
	{
		now += 10;
		server.tick();
		a.client.update(a.local);
		b.client.update(b.local);
	}
	now += 10;
	server.tick();
	b.client.update(b.local);
	CHECK(b.seen.moved.size() == 20);
	for (size_t i = 1; i < b.seen.moved.size(); ++i)
		CHECK(b.seen.moved[i].state.seq == (uint16_t)(b.seen.moved[i - 1].state.seq + 1));
}

TEST_CASE_FIXTURE(ClientFixture, "frames that do not divide fifty milliseconds still send twenty states a second")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.seen.moved.clear();
	for (int frame = 0; frame < 125; ++frame)
	{
		now += 16;
		server.tick();
		a.client.update(a.local);
		b.client.update(b.local);
	}
	now += 16;
	server.tick();
	b.client.update(b.local);
	CHECK(b.seen.moved.size() >= 39);
	CHECK(b.seen.moved.size() <= 41);
}

TEST_CASE_FIXTURE(ClientFixture, "a client on another protocol version is kicked with the reason")
{
	TestClient& a = add();
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "old", 1, PROTOCOL_VERSION + 1));
	settle();
	CHECK_FALSE(a.client.isOnline());
	CHECK_FALSE(a.client.isJoined());
	CHECK(a.seen.hasLine("version"));
}

TEST_CASE_FIXTURE(ClientFixture, "connecting to nothing stays offline with a message")
{
	TestClient& a = add();
	CHECK_FALSE(a.client.connect("localhost", 1, "mikey", 1));
	CHECK_FALSE(a.client.isOnline());
	CHECK_FALSE(a.seen.lines.empty());
}

TEST_CASE_FIXTURE(ClientFixture, "losing the server takes the client offline and clears the players")
{
	TestClient& a = joined("mikey");
	joined("dan");
	server.stop();
	settle();
	CHECK_FALSE(a.client.isOnline());
	CHECK(a.seen.hasLine("Connection Lost"));
	CHECK(a.seen.left == std::vector<uint16_t>{ 2 });
	CHECK(a.client.players().empty());
}

TEST_CASE_FIXTURE(ClientFixture, "sending while offline or before the welcome does nothing")
{
	TestClient& lone = add();
	lone.client.sendChat("nobody");
	lone.client.sendHit(1);
	lone.client.sendDied();
	lone.client.update(lone.local);
	CHECK(lone.seen.lines.empty());

	TestClient& a = joined("mikey");
	TestClient& early = add();
	REQUIRE(early.client.connect("localhost", GameServer::DEFAULT_PORT, "dan", 1));
	early.client.sendChat("too soon");
	settle();
	CHECK_FALSE(a.seen.hasLine("too soon"));
}

TEST_CASE_FIXTURE(ClientFixture, "a name longer than the protocol allows is shortened, not refused")
{
	TestClient& a = add();
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, std::string(40, 'n'), 1));
	settle();
	CHECK(a.client.isJoined());
	TestClient& b = joined("dan");
	REQUIRE(b.client.players().count(1) == 1);
	CHECK(b.client.players().at(1).name == std::string(MAX_NAME, 'n'));
}

namespace
{
	std::vector<uint8_t> typesReceived(LoopbackTransport& fakeServer, PeerId& client)
	{
		std::vector<uint8_t> types;
		NetEvent event;
		while (fakeServer.poll(event))
		{
			if (event.kind == NetEvent::Connected)
				client = event.peer;
			std::vector<Frame> frames;
			if (event.kind == NetEvent::Received && unframe(event.data, frames))
				for (size_t i = 0; i < frames.size(); ++i)
					types.push_back(frames[i].type);
		}
		return types;
	}
}

TEST_CASE("a client says nothing but hello until it is welcomed")
{
	uint32_t now = 1000;
	LoopbackNetwork network;
	LoopbackTransport fakeServer(network);
	REQUIRE(fakeServer.listen(GameServer::DEFAULT_PORT, 8));
	TestClient a(network, now);
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	PeerId client = NO_PEER;
	for (int frame = 0; frame < 10; ++frame)
	{
		now += NetClient::STATE_INTERVAL_MS;
		a.client.update(a.local);
		a.client.sendHit(2);
		a.client.sendDied();
		a.client.sendChat("hi");
	}
	CHECK(typesReceived(fakeServer, client) == std::vector<uint8_t>{ HELLO });

	fakeServer.send(client, encode(WELCOME, Welcome{ 1, {}, 0.0f, 1.0f }), Reliable);
	now += NetClient::STATE_INTERVAL_MS;
	a.client.update(a.local);
	a.client.sendHit(2);
	CHECK(typesReceived(fakeServer, client) == std::vector<uint8_t>{ STATE, HIT });
}

TEST_CASE("a kick takes the client offline even if the server does not hang up")
{
	uint32_t now = 1000;
	LoopbackNetwork network;
	LoopbackTransport fakeServer(network);
	REQUIRE(fakeServer.listen(GameServer::DEFAULT_PORT, 8));
	TestClient a(network, now);
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	a.client.update(a.local);
	PeerId client = NO_PEER;
	typesReceived(fakeServer, client);
	fakeServer.send(client, encode(WELCOME, Welcome{ 1, { { 2, "dan", 0, {} } }, 0.0f, 1.0f }), Reliable);
	fakeServer.send(client, encode(KICK, Kick{ "server full" }), Reliable);
	a.client.update(a.local);
	CHECK_FALSE(a.client.isOnline());
	CHECK(a.seen.hasLine("server full"));
	CHECK(a.seen.left == std::vector<uint16_t>{ 2 });
}

namespace
{
	std::vector<uint8_t> relayedState(uint16_t id, uint16_t seq, float x)
	{
		return encode(STATE, RelayedState{ id, State{ seq, { x, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0 } });
	}

	std::vector<uint16_t> movedSeqs(const TestClient& client)
	{
		std::vector<uint16_t> seqs;
		for (size_t i = 0; i < client.seen.moved.size(); ++i)
			seqs.push_back(client.seen.moved[i].state.seq);
		return seqs;
	}
}

TEST_CASE("a state older than the last one seen from that player is dropped")
{
	uint32_t now = 1000;
	LoopbackNetwork network;
	LoopbackTransport fakeServer(network);
	REQUIRE(fakeServer.listen(GameServer::DEFAULT_PORT, 8));
	TestClient a(network, now);
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	a.client.update(a.local);
	PeerId client = NO_PEER;
	typesReceived(fakeServer, client);
	fakeServer.send(client, encode(WELCOME, Welcome{ 1, { { 2, "dan", 0, {} }, { 3, "zed", 0, {} } }, 0.0f, 1.0f }), Reliable);
	const uint16_t order[] = { 5, 4, 5, 6 };
	for (uint16_t seq : order)
		fakeServer.send(client, relayedState(2, seq, seq), Unreliable);
	fakeServer.send(client, relayedState(3, 1, 1.0f), Unreliable);
	a.client.update(a.local);
	CHECK(movedSeqs(a) == std::vector<uint16_t>{ 5, 6, 1 });
	CHECK(a.client.players().at(2).state.pos[0] == 6.0f);
}

TEST_CASE("state sequence numbers wrap from 65535 to 0")
{
	uint32_t now = 1000;
	LoopbackNetwork network;
	LoopbackTransport fakeServer(network);
	REQUIRE(fakeServer.listen(GameServer::DEFAULT_PORT, 8));
	TestClient a(network, now);
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	a.client.update(a.local);
	PeerId client = NO_PEER;
	typesReceived(fakeServer, client);
	fakeServer.send(client, encode(WELCOME, Welcome{ 1, { { 2, "dan", 0, {} } }, 0.0f, 1.0f }), Reliable);
	fakeServer.send(client, relayedState(2, 65534, 1.0f), Unreliable);
	fakeServer.send(client, relayedState(2, 65535, 2.0f), Unreliable);
	fakeServer.send(client, relayedState(2, 0, 3.0f), Unreliable);
	fakeServer.send(client, relayedState(2, 65535, 9.0f), Unreliable);
	a.client.update(a.local);
	CHECK(movedSeqs(a) == std::vector<uint16_t>{ 65534, 65535, 0 });
}

TEST_CASE_FIXTURE(ClientFixture, "over a lossy, reordering link the remote player never moves backwards in time")
{
	TestClient& a = joined("mikey");
	TestClient& b = joined("dan");
	b.transport.setUnreliableLoss(0.3f, 0.3f, 11);
	a.seen.moved.clear();
	for (int frame = 0; frame < 200; ++frame)
	{
		now += 10;
		b.local.pos[0] = (float)frame;
		server.tick();
		a.client.update(a.local);
		b.client.update(b.local);
	}
	std::vector<uint16_t> seqs = movedSeqs(a);
	CHECK(seqs.size() > 10);
	CHECK(seqs.size() < 40);
	for (size_t i = 1; i < seqs.size(); ++i)
		CHECK(isNewerSeq(seqs[i], seqs[i - 1]));
}

TEST_CASE("after a long stall the client does not burst out the states it missed")
{
	uint32_t now = 1000;
	LoopbackNetwork network;
	LoopbackTransport fakeServer(network);
	REQUIRE(fakeServer.listen(GameServer::DEFAULT_PORT, 8));
	TestClient a(network, now);
	REQUIRE(a.client.connect("localhost", GameServer::DEFAULT_PORT, "mikey", 1));
	a.client.update(a.local);
	PeerId client = NO_PEER;
	typesReceived(fakeServer, client);
	fakeServer.send(client, encode(WELCOME, Welcome{ 1, {}, 0.0f, 1.0f }), Reliable);
	a.client.update(a.local);
	now += NetClient::STATE_INTERVAL_MS;
	a.client.update(a.local);
	typesReceived(fakeServer, client);

	now += 1000;
	for (int frame = 0; frame < 6; ++frame)
	{
		a.client.update(a.local);
		now += 10;
	}
	std::vector<uint8_t> sent = typesReceived(fakeServer, client);
	CHECK(sent.size() == 2);
}

TEST_CASE_FIXTURE(ClientFixture, "the welcome sets the joiner's clock to the server's time of day, which runs at the server's time scale")
{
	server.setClock(6.0f, 72.0f);
	TestClient& a = joined("mikey");
	REQUIRE(a.seen.clocks.size() == 1);
	CHECK(a.seen.clocks[0].first == doctest::Approx(6.0f).epsilon(0.001));
	CHECK(a.seen.clocks[0].second == 72.0f);
	advance(50000);
	TestClient& b = joined("dan");
	REQUIRE(b.seen.clocks.size() == 1);
	CHECK(b.seen.clocks[0].first == doctest::Approx(7.0f).epsilon(0.001));
	CHECK(b.seen.clocks[0].second == 72.0f);
	CHECK(a.seen.clocks.size() == 1);
}
