#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <functional>
#include <map>
#include <string>
#include <vector>
#include "net/MessageRouter.h"
#include "net/Transport.h"
#include "protocol/GameMessages.h"

class GameServer
{
public:
	typedef std::function<uint32_t()> Clock;
	typedef std::function<void(const std::string&)> Logger;

	static const uint16_t DEFAULT_PORT = 25515;
	static const int MAX_PLAYERS = 8;
	static const uint32_t STATE_INTERVAL_MS = 50;
	static const uint32_t STATE_JITTER_MS = 10;
	static const uint32_t HIT_COOLDOWN_MS = 300;
	static const int MAX_MESSAGES_PER_SECOND = 30;

	GameServer(Transport& transport, Clock clock, Logger log);

	bool start(uint16_t port, int maxPlayers = MAX_PLAYERS);
	void tick();
	void stop();

	void say(const std::string& text);
	bool kick(const std::string& name);
	std::vector<std::string> playerList() const;

private:
	struct Player
	{
		uint16_t id;
		std::string name;
		uint8_t model;
		bool hello;
		bool alive;
		bool hasPosition;
		float position[3];
		bool hasState;
		uint32_t lastStateTime;
		bool hasHit;
		uint32_t lastHitTime;
		uint32_t windowStart;
		int windowCount;
		bool rateLogged;
	};

	Player* joined(PeerId peer);
	bool withinRate(PeerId peer, Player& player);
	std::string uniqueName(const std::string& wanted) const;
	PeerId peerOf(uint16_t playerId) const;
	void sendToPlayers(const std::vector<uint8_t>& packet, Delivery delivery, PeerId except);

	void onHello(PeerId peer, ByteReader& in);
	void onState(PeerId peer, ByteReader& in);
	void onChat(PeerId peer, ByteReader& in);
	void onHit(PeerId peer, ByteReader& in);
	void onDied(PeerId peer, ByteReader& in);
	void onLeft(PeerId peer);

	Transport& transport_;
	Clock clock_;
	Logger log_;
	MessageRouter router_;
	std::map<PeerId, Player> players_;
	uint16_t nextPlayerId_;
};

#endif
