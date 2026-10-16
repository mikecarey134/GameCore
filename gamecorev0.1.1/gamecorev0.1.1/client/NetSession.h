#ifndef NETSESSION_H
#define NETSESSION_H

#include <string>
#include "client/NetClient.h"
#include "server/GameServer.h"

class NetSession
{
public:
	NetSession(Transport& serverTransport, NetClient& client, GameServer::Clock clock, GameServer::Logger log);

	bool host(uint16_t port, const std::string& name, uint8_t model, int maxPlayers = GameServer::MAX_PLAYERS);
	void setClock(float timeOfDay, float timeScale) { server_.setClock(timeOfDay, timeScale); }
	bool join(const std::string& address, uint16_t port, const std::string& name, uint8_t model);
	void tick();
	void leave();

	bool isHosting() const { return hosting_; }

private:
	GameServer server_;
	NetClient& client_;
	bool hosting_;
};

#endif
