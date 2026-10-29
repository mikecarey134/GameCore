#ifndef GAMEMESSAGES_H
#define GAMEMESSAGES_H

#include <string>
#include <vector>
#include "net/ByteStream.h"
#include "net/MessageFrame.h"

namespace protocol
{
	const uint16_t PROTOCOL_VERSION = 3;
	const size_t MAX_NAME = 32;
	const size_t MAX_TEXT = 200;
	const size_t MAX_PROFILE = 255;
	const uint8_t STATE_DEAD = 5;

	enum MessageType
	{
		HELLO = 0,
		WELCOME = 1,
		KICK = 2,
		PLAYER_JOINED = 16,
		PLAYER_LEFT = 17,
		STATE = 18,
		CHAT = 19,
		HIT = 20,
		DAMAGED = 21,
		DIED = 22,
		PLAYER_DIED = 23,
		SERVER_TEXT = 24
	};

	struct PlayerInfo { uint16_t id; std::string name; uint8_t model; std::vector<uint8_t> profile; };
	struct Hello { uint16_t protocol; std::string name; uint8_t model; std::vector<uint8_t> profile; };
	struct Welcome { uint16_t yourId; std::vector<PlayerInfo> players; float timeOfDay; float timeScale; };
	struct Kick { std::string reason; };
	struct PlayerJoined { PlayerInfo player; };
	struct PlayerLeft { uint16_t id; };
	struct State { uint16_t seq; float pos[3]; float rot[3]; uint8_t state; };
	struct RelayedState { uint16_t id; State state; };
	struct Chat { std::string text; };
	struct RelayedChat { uint16_t id; std::string text; };
	struct Hit { uint16_t targetId; };
	struct Damaged { uint16_t byId; };
	struct Died {};
	struct PlayerDied { uint16_t id; };
	struct ServerText { std::string text; };

	bool isAlive(uint8_t state);

	bool write(ByteWriter& out, const Hello& message);
	bool write(ByteWriter& out, const Welcome& message);
	bool write(ByteWriter& out, const Kick& message);
	bool write(ByteWriter& out, const PlayerJoined& message);
	bool write(ByteWriter& out, const PlayerLeft& message);
	bool write(ByteWriter& out, const State& message);
	bool write(ByteWriter& out, const RelayedState& message);
	bool write(ByteWriter& out, const Chat& message);
	bool write(ByteWriter& out, const RelayedChat& message);
	bool write(ByteWriter& out, const Hit& message);
	bool write(ByteWriter& out, const Damaged& message);
	bool write(ByteWriter& out, const Died& message);
	bool write(ByteWriter& out, const PlayerDied& message);
	bool write(ByteWriter& out, const ServerText& message);

	bool read(ByteReader& in, Hello& message);
	bool read(ByteReader& in, Welcome& message);
	bool read(ByteReader& in, Kick& message);
	bool read(ByteReader& in, PlayerJoined& message);
	bool read(ByteReader& in, PlayerLeft& message);
	bool read(ByteReader& in, State& message);
	bool read(ByteReader& in, RelayedState& message);
	bool read(ByteReader& in, Chat& message);
	bool read(ByteReader& in, RelayedChat& message);
	bool read(ByteReader& in, Hit& message);
	bool read(ByteReader& in, Damaged& message);
	bool read(ByteReader& in, Died& message);
	bool read(ByteReader& in, PlayerDied& message);
	bool read(ByteReader& in, ServerText& message);

	template <class Message>
	std::vector<uint8_t> encode(MessageType type, const Message& message)
	{
		ByteWriter out;
		if (!write(out, message))
			return std::vector<uint8_t>();
		return frameMessage((uint8_t)type, out.data());
	}
}

#endif
