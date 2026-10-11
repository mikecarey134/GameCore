#include "doctest.h"
#include "server/GameServer.h"
#include "net/LoopbackTransport.h"
#include <initializer_list>

using namespace protocol;

namespace
{
	struct Client
	{
		explicit Client(LoopbackNetwork& network) : transport(network), server(NO_PEER), disconnected(false) {}

		void connect()
		{
			REQUIRE(transport.connect("localhost", GameServer::DEFAULT_PORT));
			drain();
		}

		void drain()
		{
			NetEvent event;
			while (transport.poll(event))
			{
				if (event.kind == NetEvent::Connected)
					server = event.peer;
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
		void send(MessageType type, const Message& message, Delivery delivery = Reliable)
		{
			transport.send(server, encode(type, message), delivery);
		}

		int count(MessageType type) const
		{
			int total = 0;
			for (size_t i = 0; i < received.size(); ++i)
				if (received[i].first == type)
					++total;
			return total;
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

		LoopbackTransport transport;
		PeerId server;
		bool disconnected;
		std::vector<std::pair<uint8_t, std::vector<uint8_t> > > received;
	};

	struct ServerFixture
	{
		ServerFixture()
			: now(1000), serverTransport(network),
			  server(serverTransport, [this] { return now; }, [this](const std::string& line) { log.push_back(line); })
		{
			REQUIRE(server.start(GameServer::DEFAULT_PORT));
		}

		void settle(std::initializer_list<Client*> clients)
		{
			server.tick();
			for (Client* client : clients)
				client->drain();
		}

		uint16_t join(Client& client, const std::string& name, std::initializer_list<Client*> others = {})
		{
			client.connect();
			client.send(HELLO, Hello{ PROTOCOL_VERSION, name, 1 });
			server.tick();
			client.drain();
			for (Client* other : others)
				other->drain();
			Welcome welcome;
			REQUIRE(client.last(WELCOME, welcome));
			return welcome.yourId;
		}

		void moveTo(Client& client, float x, float z, uint8_t state = 0)
		{
			now += GameServer::STATE_INTERVAL_MS;
			client.send(STATE, State{ 1, { x, 0.0f, z }, { 0.0f, 0.0f, 0.0f }, state }, Unreliable);
		}

		uint32_t now;
		LoopbackNetwork network;
		LoopbackTransport serverTransport;
		std::vector<std::string> log;
		GameServer server;
	};
}

TEST_CASE_FIXTURE(ServerFixture, "hello is answered with a welcome carrying the assigned id")
{
	Client a(network), b(network);
	CHECK(join(a, "mikey") == 1);
	CHECK(join(b, "dan", { &a }) == 2);
}

TEST_CASE_FIXTURE(ServerFixture, "the welcome lists the players already in the game")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	Welcome welcome;
	REQUIRE(b.last(WELCOME, welcome));
	REQUIRE(welcome.players.size() == 1);
	CHECK(welcome.players[0].id == 1);
	CHECK(welcome.players[0].name == "mikey");
	CHECK(welcome.players[0].model == 1);
	REQUIRE(a.last(WELCOME, welcome));
	CHECK(welcome.players.empty());
}

TEST_CASE_FIXTURE(ServerFixture, "a new player is announced to everyone already in the game")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	PlayerJoined joined;
	REQUIRE(a.last(PLAYER_JOINED, joined));
	CHECK(joined.player.id == 2);
	CHECK(joined.player.name == "dan");
	CHECK(b.count(PLAYER_JOINED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a client on another protocol version is kicked")
{
	Client a(network);
	a.connect();
	a.send(HELLO, Hello{ (uint16_t)(PROTOCOL_VERSION + 1), "old", 0 });
	settle({ &a });
	Kick kick;
	REQUIRE(a.last(KICK, kick));
	CHECK(kick.reason.find("version") != std::string::npos);
	CHECK(a.disconnected);
	CHECK(a.count(WELCOME) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a name already in use gets a numbered suffix that still fits")
{
	Client a(network), b(network), c(network), d(network);
	join(a, "mikey");
	join(b, "mikey", { &a });
	join(c, "mikey", { &a, &b });
	std::string longName(MAX_NAME, 'z');
	join(d, longName, { &a, &b, &c });
	Client e(network);
	join(e, longName, { &a, &b, &c, &d });

	Welcome welcome;
	REQUIRE(e.last(WELCOME, welcome));
	REQUIRE(welcome.players.size() == 4);
	CHECK(welcome.players[0].name == "mikey");
	CHECK(welcome.players[1].name == "mikey_2");
	CHECK(welcome.players[2].name == "mikey_3");
	CHECK(welcome.players[3].name == longName);
	PlayerJoined joined;
	REQUIRE(a.last(PLAYER_JOINED, joined));
	CHECK(joined.player.name == std::string(MAX_NAME - 2, 'z') + "_2");
}

TEST_CASE_FIXTURE(ServerFixture, "an empty name is replaced")
{
	Client a(network), b(network);
	join(a, "");
	join(b, "x", { &a });
	Welcome welcome;
	REQUIRE(b.last(WELCOME, welcome));
	CHECK(welcome.players[0].name == "player");
}

TEST_CASE_FIXTURE(ServerFixture, "game messages before hello are dropped")
{
	Client a(network), early(network);
	join(a, "mikey");
	early.connect();
	early.send(CHAT, Chat{ "too soon" });
	early.send(DIED, Died{});
	settle({ &a, &early });
	CHECK(a.count(CHAT) == 0);
	CHECK(a.count(PLAYER_DIED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a second hello from the same client is ignored")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	b.send(HELLO, Hello{ PROTOCOL_VERSION, "again", 1 });
	settle({ &a, &b });
	CHECK(a.count(PLAYER_JOINED) == 1);
	CHECK(b.count(WELCOME) == 1);
}

TEST_CASE_FIXTURE(ServerFixture, "a state is relayed to the others with the sender's id")
{
	Client a(network), b(network), c(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	join(c, "zed", { &a, &b });
	b.send(STATE, State{ 7, { 1.0f, 2.0f, 3.0f }, { 0.0f, 45.0f, 0.0f }, 3 }, Unreliable);
	settle({ &a, &b, &c });
	RelayedState relayed;
	REQUIRE(a.last(STATE, relayed));
	CHECK(relayed.id == 2);
	CHECK(relayed.state.seq == 7);
	CHECK(relayed.state.pos[2] == 3.0f);
	CHECK(relayed.state.rot[1] == 45.0f);
	CHECK(relayed.state.state == 3);
	CHECK(c.count(STATE) == 1);
	CHECK(b.count(STATE) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "states closer than the interval less ten milliseconds of jitter are dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	b.send(STATE, State{ 1, { 0, 0, 0 }, { 0, 0, 0 }, 0 }, Unreliable);
	settle({ &a, &b });
	now += 39;
	b.send(STATE, State{ 2, { 0, 0, 0 }, { 0, 0, 0 }, 0 }, Unreliable);
	settle({ &a, &b });
	CHECK(a.count(STATE) == 1);
	now += 1;
	b.send(STATE, State{ 3, { 0, 0, 0 }, { 0, 0, 0 }, 0 }, Unreliable);
	settle({ &a, &b });
	CHECK(a.count(STATE) == 2);
	now += 45;
	b.send(STATE, State{ 4, { 0, 0, 0 }, { 0, 0, 0 }, 0 }, Unreliable);
	settle({ &a, &b });
	CHECK(a.count(STATE) == 3);
}

TEST_CASE_FIXTURE(ServerFixture, "chat is relayed to the others with the sender's id")
{
	Client a(network), b(network), waiting(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	waiting.connect();
	a.send(CHAT, Chat{ "hello all" });
	settle({ &a, &b, &waiting });
	RelayedChat chat;
	REQUIRE(b.last(CHAT, chat));
	CHECK(chat.id == 1);
	CHECK(chat.text == "hello all");
	CHECK(a.count(CHAT) == 0);
	CHECK(waiting.count(CHAT) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a malformed message is dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	ByteWriter junk;
	junk.writeU8(1);
	a.transport.send(a.server, frameMessage(CHAT, junk.data()), Reliable);
	a.transport.send(a.server, frameMessage(HIT, { 2, 0, 9 }), Reliable);
	settle({ &a, &b });
	CHECK(b.count(CHAT) == 0);
	CHECK(b.count(DAMAGED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a hit in range is sent to the target only")
{
	Client a(network), b(network), c(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	join(c, "zed", { &a, &b });
	moveTo(a, 0.0f, 0.0f);
	moveTo(b, 20.0f, 0.0f);
	settle({ &a, &b, &c });
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b, &c });
	Damaged damaged;
	REQUIRE(b.last(DAMAGED, damaged));
	CHECK(damaged.byId == 1);
	CHECK(a.count(DAMAGED) == 0);
	CHECK(c.count(DAMAGED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a hit on a target out of range is dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	moveTo(a, 0.0f, 0.0f);
	moveTo(b, 14.0f, 14.5f);
	settle({ &a, &b });
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a hit before either side has a position is dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	moveTo(a, 0.0f, 0.0f);
	settle({ &a, &b });
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "hits closer together than the cooldown are dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	moveTo(a, 0.0f, 0.0f);
	moveTo(b, 5.0f, 0.0f);
	settle({ &a, &b });
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	now += GameServer::HIT_COOLDOWN_MS - 1;
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 1);
	now += 1;
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 2);
}

TEST_CASE_FIXTURE(ServerFixture, "dead players cannot be hit and cannot hit until they respawn")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	moveTo(a, 0.0f, 0.0f);
	moveTo(b, 5.0f, 0.0f);
	settle({ &a, &b });
	b.send(DIED, Died{});
	settle({ &a, &b });
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 0);
	b.send(HIT, Hit{ 1 });
	settle({ &a, &b });
	CHECK(a.count(DAMAGED) == 0);

	moveTo(b, 5.0f, 0.0f, STATE_DEAD);
	settle({ &a, &b });
	now += GameServer::HIT_COOLDOWN_MS;
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 0);

	moveTo(b, 5.0f, 0.0f, 0);
	settle({ &a, &b });
	now += GameServer::HIT_COOLDOWN_MS;
	a.send(HIT, Hit{ 2 });
	settle({ &a, &b });
	CHECK(b.count(DAMAGED) == 1);
}

TEST_CASE_FIXTURE(ServerFixture, "a hit on yourself or on nobody is dropped")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	moveTo(a, 0.0f, 0.0f);
	moveTo(b, 5.0f, 0.0f);
	settle({ &a, &b });
	a.send(HIT, Hit{ 1 });
	a.send(HIT, Hit{ 99 });
	settle({ &a, &b });
	CHECK(a.count(DAMAGED) == 0);
	CHECK(b.count(DAMAGED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a death is announced to the others")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	b.send(DIED, Died{});
	settle({ &a, &b });
	PlayerDied died;
	REQUIRE(a.last(PLAYER_DIED, died));
	CHECK(died.id == 2);
	CHECK(b.count(PLAYER_DIED) == 0);
}

TEST_CASE_FIXTURE(ServerFixture, "a player who leaves is announced, a client who never said hello is not")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	{
		Client quiet(network);
		quiet.connect();
		settle({ &a, &b });
	}
	settle({ &a, &b });
	CHECK(a.count(PLAYER_LEFT) == 0);

	b.transport.disconnect(b.server);
	settle({ &a, &b });
	PlayerLeft left;
	REQUIRE(a.last(PLAYER_LEFT, left));
	CHECK(left.id == 2);

	Client c(network);
	CHECK(join(c, "dan", { &a }) == 3);
	Welcome welcome;
	REQUIRE(c.last(WELCOME, welcome));
	REQUIRE(welcome.players.size() == 1);
	CHECK(welcome.players[0].id == 1);
}

TEST_CASE_FIXTURE(ServerFixture, "the thirty-first message in a second is dropped and logged")
{
	Client a(network), b(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	now += 1000;
	size_t before = log.size();
	for (int i = 0; i < GameServer::MAX_MESSAGES_PER_SECOND + 2; ++i)
		a.send(CHAT, Chat{ "spam" });
	settle({ &a, &b });
	CHECK(b.count(CHAT) == GameServer::MAX_MESSAGES_PER_SECOND);
	REQUIRE(log.size() == before + 1);
	CHECK(log.back().find("in a second") != std::string::npos);

	moveTo(a, 1.0f, 1.0f);
	settle({ &a, &b });
	CHECK(b.count(STATE) == 1);

	now += 1000;
	a.send(CHAT, Chat{ "later" });
	settle({ &a, &b });
	CHECK(b.count(CHAT) == GameServer::MAX_MESSAGES_PER_SECOND + 1);
}

TEST_CASE_FIXTURE(ServerFixture, "the server can talk to, list and kick players")
{
	Client a(network), b(network), waiting(network);
	join(a, "mikey");
	join(b, "dan", { &a });
	waiting.connect();
	settle({ &a, &b, &waiting });
	CHECK_FALSE(server.kick(""));
	CHECK_FALSE(waiting.disconnected);
	server.say("restart soon");
	settle({ &a, &b });
	ServerText text;
	REQUIRE(a.last(SERVER_TEXT, text));
	CHECK(text.text == "restart soon");
	CHECK(b.count(SERVER_TEXT) == 1);

	CHECK(server.playerList() == std::vector<std::string>{ "1 mikey", "2 dan" });

	CHECK_FALSE(server.kick("nobody"));
	CHECK(server.kick("dan"));
	settle({ &a, &b });
	CHECK(b.count(KICK) == 1);
	CHECK(b.disconnected);
	PlayerLeft left;
	REQUIRE(a.last(PLAYER_LEFT, left));
	CHECK(left.id == 2);
	CHECK(server.playerList() == std::vector<std::string>{ "1 mikey" });
}

TEST_CASE_FIXTURE(ServerFixture, "stopping the server disconnects everyone")
{
	Client a(network);
	join(a, "mikey");
	server.stop();
	a.drain();
	CHECK(a.disconnected);
}
