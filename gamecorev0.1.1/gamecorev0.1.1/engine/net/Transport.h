#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <cstdint>
#include <string>
#include <vector>

typedef uint32_t PeerId;
const PeerId NO_PEER = 0;

enum Delivery { Reliable, Unreliable };

struct NetEvent
{
	enum Kind { Connected, Disconnected, Received };
	Kind kind;
	PeerId peer;
	std::vector<uint8_t> data;
};

class Transport
{
public:
	virtual ~Transport() {}
	virtual bool listen(uint16_t port, int maxPeers) = 0;
	virtual bool connect(const std::string& host, uint16_t port) = 0;
	virtual void send(PeerId peer, const std::vector<uint8_t>& data, Delivery delivery) = 0;
	virtual void broadcast(const std::vector<uint8_t>& data, Delivery delivery, PeerId exceptPeer) = 0;
	virtual void disconnect(PeerId peer) = 0;
	virtual bool poll(NetEvent& event) = 0;
	virtual void shutdown() = 0;
};

#endif
