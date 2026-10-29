#ifndef ENETTRANSPORT_H
#define ENETTRANSPORT_H

#include <map>
#include "Transport.h"

struct _ENetHost;
struct _ENetPeer;

class EnetTransport : public Transport
{
public:
	static const uint32_t DEFAULT_TIMEOUT_MS = 10000;

	explicit EnetTransport(uint32_t timeoutMs = DEFAULT_TIMEOUT_MS);
	~EnetTransport() override;

	bool listen(uint16_t port, int maxPeers) override;
	bool connect(const std::string& host, uint16_t port) override;
	void send(PeerId peer, const std::vector<uint8_t>& data, Delivery delivery) override;
	void broadcast(const std::vector<uint8_t>& data, Delivery delivery, PeerId exceptPeer) override;
	void disconnect(PeerId peer) override;
	bool poll(NetEvent& event) override;
	void shutdown() override;
	std::string address(PeerId peer) const override;
	PeerStats stats(PeerId peer) const override;

private:
	PeerId track(_ENetPeer* peer);

	_ENetHost* host_;
	std::map<PeerId, _ENetPeer*> peers_;
	std::map<PeerId, PeerStats> traffic_;
	PeerId nextPeer_;
	uint32_t timeoutMs_;
};

#endif
