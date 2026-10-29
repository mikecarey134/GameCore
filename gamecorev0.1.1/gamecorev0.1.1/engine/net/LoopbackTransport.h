#ifndef LOOPBACKTRANSPORT_H
#define LOOPBACKTRANSPORT_H

#include <deque>
#include <map>
#include <random>
#include "Transport.h"

class LoopbackTransport;

class LoopbackNetwork
{
private:
	friend class LoopbackTransport;
	std::map<uint16_t, LoopbackTransport*> listeners_;
};

class LoopbackTransport : public Transport
{
public:
	explicit LoopbackTransport(LoopbackNetwork& network);
	~LoopbackTransport() override;

	bool listen(uint16_t port, int maxPeers) override;
	bool connect(const std::string& host, uint16_t port) override;
	void send(PeerId peer, const std::vector<uint8_t>& data, Delivery delivery) override;
	void broadcast(const std::vector<uint8_t>& data, Delivery delivery, PeerId exceptPeer) override;
	void disconnect(PeerId peer) override;
	bool poll(NetEvent& event) override;
	void shutdown() override;
	std::string address(PeerId peer) const override;
	PeerStats stats(PeerId peer) const override;

	void setUnreliableLoss(float dropRate, float reorderRate, uint32_t seed);
	void setAddress(const std::string& address) { address_ = address; }

private:
	struct Link
	{
		LoopbackTransport* other;
		PeerId remoteId;
		uint64_t bytesSent;
		uint64_t bytesReceived;
	};

	PeerId addLink(LoopbackTransport* other, PeerId remoteId);
	void push(NetEvent::Kind kind, PeerId peer, const std::vector<uint8_t>& data);

	LoopbackNetwork& network_;
	std::map<PeerId, Link> links_;
	std::deque<NetEvent> events_;
	PeerId nextPeer_;
	int maxPeers_;
	uint16_t port_;
	bool listening_;
	float dropRate_;
	float reorderRate_;
	std::mt19937 random_;
	std::map<PeerId, std::vector<uint8_t> > held_;
	std::string address_;
};

#endif
