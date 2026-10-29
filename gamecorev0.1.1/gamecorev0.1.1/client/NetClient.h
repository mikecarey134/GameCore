#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <functional>
#include <map>
#include <string>
#include "net/MessageRouter.h"
#include "net/Transport.h"
#include "protocol/GameMessages.h"

class NetClient
{
public:
	typedef std::function<uint32_t()> Clock;

	enum LineKind { ChatLine, JoinedLine, LeftLine, DiedLine, ServerLine, InfoLine, ErrorLine };

	struct Remote
	{
		uint16_t id;
		std::string name;
		uint8_t model;
		std::vector<uint8_t> profile;
		bool alive;
		bool hasState;
		protocol::State state;
	};

	struct Events
	{
		std::function<void(const std::string&, LineKind)> line;
		std::function<void(const Remote&)> joined;
		std::function<void(uint16_t)> left;
		std::function<void(const Remote&)> moved;
		std::function<void(uint16_t)> died;
		std::function<void(const Remote&)> respawned;
		std::function<void(uint16_t)> damaged;
		std::function<void(float timeOfDay, float timeScale)> clock;
	};

	static const uint32_t STATE_INTERVAL_MS = 50;

	NetClient(Transport& transport, Clock clock, Events events);

	bool connect(const std::string& host, uint16_t port, const std::string& name, uint8_t model,
		uint16_t protocolVersion = protocol::PROTOCOL_VERSION);
	void update(const protocol::State& local);
	void setProfile(const std::vector<uint8_t>& profile) { profile_ = profile; }
	void sendChat(const std::string& text);
	void sendHit(uint16_t targetId);
	void sendDied();
	void shutdown();

	bool isOnline() const { return online_; }
	bool isJoined() const { return joined_; }
	uint16_t localId() const { return localId_; }
	const std::map<uint16_t, Remote>& players() const { return players_; }

private:
	void say(const std::string& text, LineKind kind);
	void goOffline();
	void addRemote(const protocol::PlayerInfo& info);
	std::string nameOf(uint16_t id) const;

	void onWelcome(ByteReader& in);
	void onPlayerJoined(ByteReader& in);
	void onPlayerLeft(ByteReader& in);
	void onState(ByteReader& in);
	void onChat(ByteReader& in);
	void onDamaged(ByteReader& in);
	void onPlayerDied(ByteReader& in);
	void onKick(ByteReader& in);
	void onServerText(ByteReader& in);

	Transport& transport_;
	Clock clock_;
	Events events_;
	MessageRouter router_;
	std::map<uint16_t, Remote> players_;
	PeerId server_;
	bool online_;
	bool connected_;
	bool joined_;
	uint16_t localId_;
	std::string name_;
	uint8_t model_;
	std::vector<uint8_t> profile_;
	uint16_t protocolVersion_;
	bool sentState_;
	uint32_t lastStateTime_;
	uint16_t stateSeq_;
};

#endif
