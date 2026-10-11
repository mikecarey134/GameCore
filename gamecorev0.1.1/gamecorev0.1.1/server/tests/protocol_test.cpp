#include "doctest.h"
#include "protocol/GameMessages.h"

using namespace protocol;

namespace
{
	template <class Message>
	bool decode(const std::vector<uint8_t>& packet, MessageType expectedType, Message& message)
	{
		std::vector<Frame> frames;
		if (!unframe(packet, frames) || frames.size() != 1 || frames[0].type != expectedType)
			return false;
		ByteReader in(frames[0].payload, frames[0].size);
		return read(in, message);
	}

	template <class Message>
	bool readBytes(const std::vector<uint8_t>& payload, Message& message)
	{
		ByteReader in(payload);
		return read(in, message);
	}
}

TEST_CASE("message type numbers are fixed")
{
	CHECK(HELLO == 0);
	CHECK(WELCOME == 1);
	CHECK(KICK == 2);
	CHECK(PLAYER_JOINED == 16);
	CHECK(PLAYER_LEFT == 17);
	CHECK(STATE == 18);
	CHECK(CHAT == 19);
	CHECK(HIT == 20);
	CHECK(DAMAGED == 21);
	CHECK(DIED == 22);
	CHECK(PLAYER_DIED == 23);
	CHECK(SERVER_TEXT == 24);
	CHECK(PROTOCOL_VERSION == 1);
}

TEST_CASE("hello is written as version, name and model")
{
	ByteWriter out;
	CHECK(write(out, Hello{ 1, "mikey", 2 }));
	CHECK(out.data() == std::vector<uint8_t>{ 1, 0, 5, 0, 'm', 'i', 'k', 'e', 'y', 2 });

	Hello hello;
	REQUIRE(decode(encode(HELLO, Hello{ 7, "dan", 3 }), HELLO, hello));
	CHECK(hello.protocol == 7);
	CHECK(hello.name == "dan");
	CHECK(hello.model == 3);
}

TEST_CASE("welcome carries the new id and every existing player")
{
	Welcome sent{ 4, { { 1, "mikey", 1 }, { 3, "dan", 0 } } };
	ByteWriter out;
	REQUIRE(write(out, sent));
	CHECK(out.data()[2] == 2);

	Welcome welcome;
	REQUIRE(decode(encode(WELCOME, sent), WELCOME, welcome));
	CHECK(welcome.yourId == 4);
	REQUIRE(welcome.players.size() == 2);
	CHECK(welcome.players[0].id == 1);
	CHECK(welcome.players[0].name == "mikey");
	CHECK(welcome.players[0].model == 1);
	CHECK(welcome.players[1].id == 3);
	CHECK(welcome.players[1].name == "dan");

	Welcome alone;
	REQUIRE(decode(encode(WELCOME, Welcome{ 1, {} }), WELCOME, alone));
	CHECK(alone.yourId == 1);
	CHECK(alone.players.empty());
}

TEST_CASE("state round-trips with and without the sender's id")
{
	State sent{ 9, { 1.5f, -2.0f, 3.25f }, { 0.0f, 90.0f, 0.0f }, 3 };
	State state;
	REQUIRE(decode(encode(STATE, sent), STATE, state));
	CHECK(state.seq == 9);
	CHECK(state.pos[0] == 1.5f);
	CHECK(state.pos[1] == -2.0f);
	CHECK(state.pos[2] == 3.25f);
	CHECK(state.rot[1] == 90.0f);
	CHECK(state.state == 3);

	RelayedState relayed;
	REQUIRE(decode(encode(STATE, RelayedState{ 5, sent }), STATE, relayed));
	CHECK(relayed.id == 5);
	CHECK(relayed.state.seq == 9);
	CHECK(relayed.state.pos[2] == 3.25f);
	CHECK(relayed.state.rot[1] == 90.0f);
	CHECK(relayed.state.state == 3);
}

TEST_CASE("the small messages round-trip")
{
	Kick kick;
	REQUIRE(decode(encode(KICK, Kick{ "bye" }), KICK, kick));
	CHECK(kick.reason == "bye");

	PlayerJoined joined;
	REQUIRE(decode(encode(PLAYER_JOINED, PlayerJoined{ { 6, "zed", 2 } }), PLAYER_JOINED, joined));
	CHECK(joined.player.id == 6);
	CHECK(joined.player.name == "zed");
	CHECK(joined.player.model == 2);

	PlayerLeft left;
	REQUIRE(decode(encode(PLAYER_LEFT, PlayerLeft{ 6 }), PLAYER_LEFT, left));
	CHECK(left.id == 6);

	Chat chat;
	REQUIRE(decode(encode(CHAT, Chat{ "hello all" }), CHAT, chat));
	CHECK(chat.text == "hello all");

	RelayedChat relayedChat;
	REQUIRE(decode(encode(CHAT, RelayedChat{ 2, "hi" }), CHAT, relayedChat));
	CHECK(relayedChat.id == 2);
	CHECK(relayedChat.text == "hi");

	Hit hit;
	REQUIRE(decode(encode(HIT, Hit{ 8 }), HIT, hit));
	CHECK(hit.targetId == 8);

	Damaged damaged;
	REQUIRE(decode(encode(DAMAGED, Damaged{ 3 }), DAMAGED, damaged));
	CHECK(damaged.byId == 3);

	Died died;
	REQUIRE(decode(encode(DIED, Died{}), DIED, died));
	CHECK(encode(DIED, Died{}) == std::vector<uint8_t>{ DIED, 0, 0 });

	PlayerDied playerDied;
	REQUIRE(decode(encode(PLAYER_DIED, PlayerDied{ 4 }), PLAYER_DIED, playerDied));
	CHECK(playerDied.id == 4);

	ServerText text;
	REQUIRE(decode(encode(SERVER_TEXT, ServerText{ "restart soon" }), SERVER_TEXT, text));
	CHECK(text.text == "restart soon");
}

TEST_CASE("names and texts over their limits are refused when written and when read")
{
	std::string longName(MAX_NAME + 1, 'x');
	CHECK(encode(HELLO, Hello{ 1, longName, 0 }).empty());
	CHECK(encode(PLAYER_JOINED, PlayerJoined{ { 1, longName, 0 } }).empty());
	CHECK(encode(WELCOME, Welcome{ 1, { { 2, longName, 0 } } }).empty());
	CHECK_FALSE(encode(HELLO, Hello{ 1, std::string(MAX_NAME, 'x'), 0 }).empty());
	CHECK(encode(CHAT, Chat{ std::string(MAX_TEXT + 1, 'y') }).empty());
	CHECK_FALSE(encode(CHAT, Chat{ std::string(MAX_TEXT, 'y') }).empty());

	ByteWriter forged;
	forged.writeU16(1);
	forged.writeString(longName, 1000);
	forged.writeU8(0);
	Hello hello;
	CHECK_FALSE(readBytes(forged.data(), hello));

	ByteWriter forgedChat;
	forgedChat.writeString(std::string(MAX_TEXT + 1, 'y'), 1000);
	Chat chat;
	CHECK_FALSE(readBytes(forgedChat.data(), chat));
}

TEST_CASE("a truncated message does not read")
{
	std::vector<uint8_t> payload = { 1, 0, 3, 0, 'd', 'a' };
	Hello hello;
	CHECK_FALSE(readBytes(payload, hello));

	std::vector<uint8_t> shortState(10, 0);
	State state;
	CHECK_FALSE(readBytes(shortState, state));

	std::vector<uint8_t> welcomeMissingPlayer = { 2, 0, 1 };
	Welcome welcome;
	CHECK_FALSE(readBytes(welcomeMissingPlayer, welcome));
}

TEST_CASE("extra bytes after a message make it unreadable")
{
	std::vector<uint8_t> hit = { 8, 0, 99 };
	Hit message;
	CHECK_FALSE(readBytes(hit, message));

	std::vector<uint8_t> died = { 0 };
	Died dead;
	CHECK_FALSE(readBytes(died, dead));
}

TEST_CASE("states below dead count as alive")
{
	CHECK(isAlive(0));
	CHECK(isAlive(4));
	CHECK_FALSE(isAlive(STATE_DEAD));
	CHECK_FALSE(isAlive(5));
	CHECK_FALSE(isAlive(6));
}
