#include "GameServer.h"

using namespace protocol;

const uint16_t GameServer::DEFAULT_PORT;
const int GameServer::MAX_PLAYERS;
const uint32_t GameServer::STATE_INTERVAL_MS;
const uint32_t GameServer::HIT_COOLDOWN_MS;
const int GameServer::MAX_MESSAGES_PER_SECOND;

namespace
{
	const float HIT_RANGE = 15.0f;
	const float HIT_TOLERANCE = 5.0f;
}

GameServer::GameServer(Transport& transport, Clock clock, Logger log)
	: transport_(transport), clock_(clock), log_(log), router_(log), nextPlayerId_(1)
{
	router_.on(HELLO, [this](PeerId peer, ByteReader& in) { onHello(peer, in); });
	router_.on(STATE, [this](PeerId peer, ByteReader& in) { onState(peer, in); });
	router_.on(CHAT, [this](PeerId peer, ByteReader& in) { onChat(peer, in); });
	router_.on(HIT, [this](PeerId peer, ByteReader& in) { onHit(peer, in); });
	router_.on(DIED, [this](PeerId peer, ByteReader& in) { onDied(peer, in); });
}

bool GameServer::start(uint16_t port, int maxPlayers)
{
	return transport_.listen(port, maxPlayers);
}

void GameServer::tick()
{
	NetEvent event;
	while (transport_.poll(event))
	{
		if (event.kind == NetEvent::Connected)
		{
			Player player = Player();
			player.alive = true;
			players_[event.peer] = player;
		}
		else if (event.kind == NetEvent::Disconnected)
			onLeft(event.peer);
		else
			router_.route(event.peer, event.data);
	}
}

void GameServer::stop()
{
	transport_.shutdown();
	players_.clear();
}

void GameServer::say(const std::string& text)
{
	sendToPlayers(encode(SERVER_TEXT, ServerText{ text.substr(0, MAX_TEXT) }), Reliable, NO_PEER);
}

bool GameServer::kick(const std::string& name)
{
	for (std::map<PeerId, Player>::iterator player = players_.begin(); player != players_.end(); ++player)
		if (player->second.hello && player->second.name == name)
		{
			transport_.send(player->first, encode(KICK, Kick{ "Kicked by the server" }), Reliable);
			transport_.disconnect(player->first);
			return true;
		}
	return false;
}

std::vector<std::string> GameServer::playerList() const
{
	std::vector<std::string> list;
	for (std::map<PeerId, Player>::const_iterator player = players_.begin(); player != players_.end(); ++player)
		if (player->second.hello)
			list.push_back(std::to_string(player->second.id) + " " + player->second.name);
	return list;
}

GameServer::Player* GameServer::joined(PeerId peer)
{
	std::map<PeerId, Player>::iterator player = players_.find(peer);
	if (player == players_.end() || !player->second.hello)
		return 0;
	return &player->second;
}

bool GameServer::withinRate(PeerId peer, Player& player)
{
	uint32_t now = clock_();
	if (player.windowCount == 0 || now - player.windowStart >= 1000)
	{
		player.windowStart = now;
		player.windowCount = 0;
		player.rateLogged = false;
	}
	if (++player.windowCount <= MAX_MESSAGES_PER_SECOND)
		return true;
	if (!player.rateLogged)
		log_("Peer " + std::to_string(peer) + " sent more than " + std::to_string(MAX_MESSAGES_PER_SECOND) + " messages in a second; dropping the rest");
	player.rateLogged = true;
	return false;
}

std::string GameServer::uniqueName(const std::string& wanted) const
{
	std::string base = wanted.empty() ? "player" : wanted;
	for (int number = 1;; ++number)
	{
		std::string candidate = base;
		if (number > 1)
		{
			std::string suffix = "_" + std::to_string(number);
			candidate = base.substr(0, MAX_NAME - suffix.size()) + suffix;
		}
		bool taken = false;
		for (std::map<PeerId, Player>::const_iterator player = players_.begin(); player != players_.end(); ++player)
			if (player->second.hello && player->second.name == candidate)
				taken = true;
		if (!taken)
			return candidate;
	}
}

PeerId GameServer::peerOf(uint16_t playerId) const
{
	for (std::map<PeerId, Player>::const_iterator player = players_.begin(); player != players_.end(); ++player)
		if (player->second.hello && player->second.id == playerId)
			return player->first;
	return NO_PEER;
}

void GameServer::sendToPlayers(const std::vector<uint8_t>& packet, Delivery delivery, PeerId except)
{
	for (std::map<PeerId, Player>::iterator player = players_.begin(); player != players_.end(); ++player)
		if (player->second.hello && player->first != except)
			transport_.send(player->first, packet, delivery);
}

void GameServer::onHello(PeerId peer, ByteReader& in)
{
	std::map<PeerId, Player>::iterator found = players_.find(peer);
	if (found == players_.end() || found->second.hello || !withinRate(peer, found->second))
		return;
	Hello hello;
	if (!read(in, hello))
		return;
	if (hello.protocol != PROTOCOL_VERSION)
	{
		transport_.send(peer, encode(KICK, Kick{ "Wrong game version: the server speaks protocol " + std::to_string(PROTOCOL_VERSION) +
			", this client speaks " + std::to_string(hello.protocol) }), Reliable);
		transport_.disconnect(peer);
		return;
	}

	Welcome welcome;
	for (std::map<PeerId, Player>::iterator other = players_.begin(); other != players_.end(); ++other)
		if (other->second.hello)
			welcome.players.push_back(PlayerInfo{ other->second.id, other->second.name, other->second.model });

	Player& player = found->second;
	player.name = uniqueName(hello.name);
	player.model = hello.model;
	player.id = nextPlayerId_++;
	player.hello = true;
	welcome.yourId = player.id;
	transport_.send(peer, encode(WELCOME, welcome), Reliable);
	sendToPlayers(encode(PLAYER_JOINED, PlayerJoined{ PlayerInfo{ player.id, player.name, player.model } }), Reliable, peer);
	log_(player.name + " joined as player " + std::to_string(player.id));
}

void GameServer::onState(PeerId peer, ByteReader& in)
{
	Player* player = joined(peer);
	State state;
	if (!player || !read(in, state))
		return;
	uint32_t now = clock_();
	if (player->hasState && now - player->lastStateTime < STATE_INTERVAL_MS)
		return;
	player->hasState = true;
	player->lastStateTime = now;
	player->hasPosition = true;
	for (int i = 0; i < 3; ++i)
		player->position[i] = state.pos[i];
	player->alive = isAlive(state.state);
	sendToPlayers(encode(STATE, RelayedState{ player->id, state }), Unreliable, peer);
}

void GameServer::onChat(PeerId peer, ByteReader& in)
{
	Player* player = joined(peer);
	Chat chat;
	if (!player || !withinRate(peer, *player) || !read(in, chat) || chat.text.empty())
		return;
	sendToPlayers(encode(CHAT, RelayedChat{ player->id, chat.text }), Reliable, peer);
}

void GameServer::onHit(PeerId peer, ByteReader& in)
{
	Player* attacker = joined(peer);
	Hit hit;
	if (!attacker || !withinRate(peer, *attacker) || !read(in, hit))
		return;
	PeerId targetPeer = peerOf(hit.targetId);
	if (targetPeer == NO_PEER || targetPeer == peer)
		return;
	Player& target = players_[targetPeer];
	if (!attacker->alive || !target.alive || !attacker->hasPosition || !target.hasPosition)
		return;
	float distanceSquared = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		float delta = attacker->position[i] - target.position[i];
		distanceSquared += delta * delta;
	}
	float reach = HIT_RANGE + HIT_TOLERANCE;
	if (distanceSquared > reach * reach)
		return;
	uint32_t now = clock_();
	if (attacker->hasHit && now - attacker->lastHitTime < HIT_COOLDOWN_MS)
		return;
	attacker->hasHit = true;
	attacker->lastHitTime = now;
	transport_.send(targetPeer, encode(DAMAGED, Damaged{ attacker->id }), Reliable);
}

void GameServer::onDied(PeerId peer, ByteReader& in)
{
	Player* player = joined(peer);
	Died died;
	if (!player || !withinRate(peer, *player) || !read(in, died))
		return;
	player->alive = false;
	sendToPlayers(encode(PLAYER_DIED, PlayerDied{ player->id }), Reliable, peer);
}

void GameServer::onLeft(PeerId peer)
{
	std::map<PeerId, Player>::iterator player = players_.find(peer);
	if (player == players_.end())
		return;
	bool announced = player->second.hello;
	uint16_t id = player->second.id;
	std::string name = player->second.name;
	players_.erase(player);
	if (announced)
	{
		sendToPlayers(encode(PLAYER_LEFT, PlayerLeft{ id }), Reliable, NO_PEER);
		log_(name + " left");
	}
}
