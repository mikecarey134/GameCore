#include "GameMessages.h"

namespace protocol
{
	namespace
	{
		bool writeProfile(ByteWriter& out, const std::vector<uint8_t>& profile)
		{
			if (profile.size() > MAX_PROFILE)
				return false;
			out.writeU8((uint8_t)profile.size());
			for (uint8_t byte : profile)
				out.writeU8(byte);
			return true;
		}

		std::vector<uint8_t> readProfile(ByteReader& in)
		{
			std::vector<uint8_t> profile(in.readU8());
			for (uint8_t& byte : profile)
				byte = in.readU8();
			return profile;
		}

		bool writePlayer(ByteWriter& out, const PlayerInfo& player)
		{
			out.writeU16(player.id);
			if (!out.writeString(player.name, MAX_NAME))
				return false;
			out.writeU8(player.model);
			return writeProfile(out, player.profile);
		}

		void readPlayer(ByteReader& in, PlayerInfo& player)
		{
			player.id = in.readU16();
			player.name = in.readString(MAX_NAME);
			player.model = in.readU8();
			player.profile = readProfile(in);
		}

		void writeState(ByteWriter& out, const State& state)
		{
			out.writeU16(state.seq);
			for (int i = 0; i < 3; ++i)
				out.writeF32(state.pos[i]);
			for (int i = 0; i < 3; ++i)
				out.writeF32(state.rot[i]);
			out.writeU8(state.state);
		}

		void readState(ByteReader& in, State& state)
		{
			state.seq = in.readU16();
			for (int i = 0; i < 3; ++i)
				state.pos[i] = in.readF32();
			for (int i = 0; i < 3; ++i)
				state.rot[i] = in.readF32();
			state.state = in.readU8();
		}
	}

	bool isAlive(uint8_t state)
	{
		return state < STATE_DEAD;
	}

	bool write(ByteWriter& out, const Hello& message)
	{
		out.writeU16(message.protocol);
		if (!out.writeString(message.name, MAX_NAME))
			return false;
		out.writeU8(message.model);
		return writeProfile(out, message.profile);
	}

	bool write(ByteWriter& out, const Welcome& message)
	{
		if (message.players.size() > 255)
			return false;
		out.writeU16(message.yourId);
		out.writeU8((uint8_t)message.players.size());
		for (size_t i = 0; i < message.players.size(); ++i)
			if (!writePlayer(out, message.players[i]))
				return false;
		return true;
	}

	bool write(ByteWriter& out, const Kick& message) { return out.writeString(message.reason, MAX_TEXT); }
	bool write(ByteWriter& out, const PlayerJoined& message) { return writePlayer(out, message.player); }
	bool write(ByteWriter& out, const PlayerLeft& message) { out.writeU16(message.id); return true; }
	bool write(ByteWriter& out, const State& message) { writeState(out, message); return true; }

	bool write(ByteWriter& out, const RelayedState& message)
	{
		out.writeU16(message.id);
		writeState(out, message.state);
		return true;
	}

	bool write(ByteWriter& out, const Chat& message) { return out.writeString(message.text, MAX_TEXT); }

	bool write(ByteWriter& out, const RelayedChat& message)
	{
		out.writeU16(message.id);
		return out.writeString(message.text, MAX_TEXT);
	}

	bool write(ByteWriter& out, const Hit& message) { out.writeU16(message.targetId); return true; }
	bool write(ByteWriter& out, const Damaged& message) { out.writeU16(message.byId); return true; }
	bool write(ByteWriter&, const Died&) { return true; }
	bool write(ByteWriter& out, const PlayerDied& message) { out.writeU16(message.id); return true; }
	bool write(ByteWriter& out, const ServerText& message) { return out.writeString(message.text, MAX_TEXT); }

	bool read(ByteReader& in, Hello& message)
	{
		message.protocol = in.readU16();
		message.name = in.readString(MAX_NAME);
		message.model = in.readU8();
		message.profile = readProfile(in);
		return !in.bad();
	}

	bool read(ByteReader& in, Welcome& message)
	{
		message.yourId = in.readU16();
		uint8_t count = in.readU8();
		message.players.resize(count);
		for (uint8_t i = 0; i < count; ++i)
			readPlayer(in, message.players[i]);
		return !in.bad();
	}

	bool read(ByteReader& in, Kick& message) { message.reason = in.readString(MAX_TEXT); return !in.bad(); }
	bool read(ByteReader& in, PlayerJoined& message) { readPlayer(in, message.player); return !in.bad(); }
	bool read(ByteReader& in, PlayerLeft& message) { message.id = in.readU16(); return !in.bad(); }
	bool read(ByteReader& in, State& message) { readState(in, message); return !in.bad(); }

	bool read(ByteReader& in, RelayedState& message)
	{
		message.id = in.readU16();
		readState(in, message.state);
		return !in.bad();
	}

	bool read(ByteReader& in, Chat& message) { message.text = in.readString(MAX_TEXT); return !in.bad(); }

	bool read(ByteReader& in, RelayedChat& message)
	{
		message.id = in.readU16();
		message.text = in.readString(MAX_TEXT);
		return !in.bad();
	}

	bool read(ByteReader& in, Hit& message) { message.targetId = in.readU16(); return !in.bad(); }
	bool read(ByteReader& in, Damaged& message) { message.byId = in.readU16(); return !in.bad(); }
	bool read(ByteReader& in, Died&) { return !in.bad(); }
	bool read(ByteReader& in, PlayerDied& message) { message.id = in.readU16(); return !in.bad(); }
	bool read(ByteReader& in, ServerText& message) { message.text = in.readString(MAX_TEXT); return !in.bad(); }
}
