#include "NetClient.h"
#include "net/Interpolator.h"

using namespace protocol;

const uint32_t NetClient::STATE_INTERVAL_MS;

NetClient::NetClient(Transport& transport, Clock clock, Events events)
	: transport_(transport), clock_(clock), events_(events), router_([](const std::string&) {}), server_(NO_PEER),
	  online_(false), connected_(false), joined_(false), localId_(0), model_(0), protocolVersion_(PROTOCOL_VERSION),
	  sentState_(false), lastStateTime_(0), stateSeq_(0)
{
	router_.on(WELCOME, [this](PeerId, ByteReader& in) { onWelcome(in); });
	router_.on(PLAYER_JOINED, [this](PeerId, ByteReader& in) { onPlayerJoined(in); });
	router_.on(PLAYER_LEFT, [this](PeerId, ByteReader& in) { onPlayerLeft(in); });
	router_.on(STATE, [this](PeerId, ByteReader& in) { onState(in); });
	router_.on(CHAT, [this](PeerId, ByteReader& in) { onChat(in); });
	router_.on(DAMAGED, [this](PeerId, ByteReader& in) { onDamaged(in); });
	router_.on(PLAYER_DIED, [this](PeerId, ByteReader& in) { onPlayerDied(in); });
	router_.on(KICK, [this](PeerId, ByteReader& in) { onKick(in); });
	router_.on(SERVER_TEXT, [this](PeerId, ByteReader& in) { onServerText(in); });
}

bool NetClient::connect(const std::string& host, uint16_t port, const std::string& name, uint8_t model, uint16_t protocolVersion)
{
	name_ = name.substr(0, MAX_NAME);
	model_ = model;
	protocolVersion_ = protocolVersion;
	if (!transport_.connect(host, port))
	{
		say("[info] Could not start connecting, playing offline", ErrorLine);
		return false;
	}
	online_ = true;
	return true;
}

void NetClient::update(const State& local)
{
	NetEvent event;
	while (online_ && transport_.poll(event))
	{
		if (event.kind == NetEvent::Connected)
		{
			server_ = event.peer;
			connected_ = true;
			transport_.send(server_, encode(HELLO, Hello{ protocolVersion_, name_, model_, profile_ }), Reliable);
		}
		else if (event.kind == NetEvent::Disconnected)
		{
			say(connected_ ? "[info] Connection Lost!" : "[info] Connection Attempt Failed!", ErrorLine);
			goOffline();
		}
		else
			router_.route(event.peer, event.data);
	}

	if (!joined_)
		return;
	uint32_t now = clock_();
	if (sentState_ && now - lastStateTime_ < STATE_INTERVAL_MS)
		return;
	lastStateTime_ = sentState_ && now - lastStateTime_ < 2 * STATE_INTERVAL_MS ? lastStateTime_ + STATE_INTERVAL_MS : now;
	sentState_ = true;
	State state = local;
	state.seq = stateSeq_++;
	transport_.send(server_, encode(STATE, state), Unreliable);
}

void NetClient::sendChat(const std::string& text)
{
	if (!joined_ || text.empty())
		return;
	std::string message = text.substr(0, MAX_TEXT);
	transport_.send(server_, encode(CHAT, Chat{ message }), Reliable);
	say(name_ + ": " + message, ChatLine);
}

void NetClient::sendHit(uint16_t targetId)
{
	if (joined_)
		transport_.send(server_, encode(HIT, Hit{ targetId }), Reliable);
}

void NetClient::sendDied()
{
	if (joined_)
		transport_.send(server_, encode(DIED, Died{}), Reliable);
}

void NetClient::shutdown()
{
	goOffline();
}

void NetClient::say(const std::string& text, LineKind kind)
{
	if (events_.line)
		events_.line(text, kind);
}

void NetClient::goOffline()
{
	transport_.shutdown();
	online_ = false;
	connected_ = false;
	joined_ = false;
	server_ = NO_PEER;
	std::map<uint16_t, Remote> leaving;
	leaving.swap(players_);
	for (std::map<uint16_t, Remote>::iterator remote = leaving.begin(); remote != leaving.end(); ++remote)
		if (events_.left)
			events_.left(remote->first);
}

void NetClient::addRemote(const PlayerInfo& info)
{
	Remote remote = Remote();
	remote.id = info.id;
	remote.name = info.name;
	remote.model = info.model;
	remote.profile = info.profile;
	remote.alive = true;
	players_[info.id] = remote;
	if (events_.joined)
		events_.joined(remote);
}

std::string NetClient::nameOf(uint16_t id) const
{
	std::map<uint16_t, Remote>::const_iterator remote = players_.find(id);
	return remote == players_.end() ? "player " + std::to_string(id) : remote->second.name;
}

void NetClient::onWelcome(ByteReader& in)
{
	Welcome welcome;
	if (joined_ || !read(in, welcome))
		return;
	joined_ = true;
	localId_ = welcome.yourId;
	say("[info] Connected To Server", InfoLine);
	for (size_t i = 0; i < welcome.players.size(); ++i)
		addRemote(welcome.players[i]);
}

void NetClient::onPlayerJoined(ByteReader& in)
{
	PlayerJoined joined;
	if (!joined_ || !read(in, joined) || players_.count(joined.player.id))
		return;
	addRemote(joined.player);
	say(joined.player.name + " Joined", JoinedLine);
}

void NetClient::onPlayerLeft(ByteReader& in)
{
	PlayerLeft left;
	if (!read(in, left))
		return;
	std::map<uint16_t, Remote>::iterator remote = players_.find(left.id);
	if (remote == players_.end())
		return;
	say(remote->second.name + " Left", LeftLine);
	players_.erase(remote);
	if (events_.left)
		events_.left(left.id);
}

void NetClient::onState(ByteReader& in)
{
	RelayedState relayed;
	if (!read(in, relayed))
		return;
	std::map<uint16_t, Remote>::iterator found = players_.find(relayed.id);
	if (found == players_.end())
		return;
	Remote& remote = found->second;
	if (remote.hasState && !isNewerSeq(relayed.state.seq, remote.state.seq))
		return;
	remote.state = relayed.state;
	remote.hasState = true;
	if (!remote.alive)
	{
		if (!isAlive(relayed.state.state))
			return;
		remote.alive = true;
		if (events_.respawned)
			events_.respawned(remote);
	}
	if (events_.moved)
		events_.moved(remote);
}

void NetClient::onChat(ByteReader& in)
{
	RelayedChat chat;
	if (read(in, chat))
		say(nameOf(chat.id) + ": " + chat.text, ChatLine);
}

void NetClient::onDamaged(ByteReader& in)
{
	Damaged damaged;
	if (read(in, damaged) && events_.damaged)
		events_.damaged(damaged.byId);
}

void NetClient::onPlayerDied(ByteReader& in)
{
	PlayerDied died;
	if (!read(in, died))
		return;
	std::map<uint16_t, Remote>::iterator remote = players_.find(died.id);
	if (remote == players_.end() || !remote->second.alive)
		return;
	remote->second.alive = false;
	say(remote->second.name + " Died", DiedLine);
	if (events_.died)
		events_.died(died.id);
}

void NetClient::onKick(ByteReader& in)
{
	Kick kick;
	if (!read(in, kick))
		return;
	say("[info] Kicked: " + kick.reason, ErrorLine);
	goOffline();
}

void NetClient::onServerText(ByteReader& in)
{
	ServerText text;
	if (read(in, text))
		say("[Server] " + text.text, ServerLine);
}
