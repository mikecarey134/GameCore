#include "NetSession.h"

NetSession::NetSession(Transport& serverTransport, NetClient& client, GameServer::Clock clock, GameServer::Logger log)
	: server_(serverTransport, clock, log), client_(client), hosting_(false)
{
}

bool NetSession::host(uint16_t port, const std::string& name, uint8_t model)
{
	leave();
	if (!server_.start(port))
		return false;
	hosting_ = true;
	if (client_.connect("127.0.0.1", port, name, model))
		return true;
	leave();
	return false;
}

bool NetSession::join(const std::string& address, uint16_t port, const std::string& name, uint8_t model)
{
	leave();
	return client_.connect(address, port, name, model);
}

void NetSession::tick()
{
	if (hosting_)
		server_.tick();
}

void NetSession::leave()
{
	client_.shutdown();
	if (hosting_)
		server_.stop();
	hosting_ = false;
}
