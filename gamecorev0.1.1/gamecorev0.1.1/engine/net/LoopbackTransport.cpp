#include "LoopbackTransport.h"

LoopbackTransport::LoopbackTransport(LoopbackNetwork& network)
	: network_(network), nextPeer_(1), maxPeers_(0), port_(0), listening_(false)
{
}

LoopbackTransport::~LoopbackTransport()
{
	shutdown();
}

bool LoopbackTransport::listen(uint16_t port, int maxPeers)
{
	if (listening_ || network_.listeners_.count(port))
		return false;
	network_.listeners_[port] = this;
	port_ = port;
	maxPeers_ = maxPeers;
	listening_ = true;
	return true;
}

bool LoopbackTransport::connect(const std::string&, uint16_t port)
{
	std::map<uint16_t, LoopbackTransport*>::iterator listener = network_.listeners_.find(port);
	if (listener == network_.listeners_.end())
		return false;
	LoopbackTransport* server = listener->second;
	if ((int)server->links_.size() >= server->maxPeers_)
		return false;

	PeerId serverOnClient = addLink(server, NO_PEER);
	PeerId clientOnServer = server->addLink(this, serverOnClient);
	links_[serverOnClient].remoteId = clientOnServer;
	push(NetEvent::Connected, serverOnClient, std::vector<uint8_t>());
	server->push(NetEvent::Connected, clientOnServer, std::vector<uint8_t>());
	return true;
}

void LoopbackTransport::send(PeerId peer, const std::vector<uint8_t>& data, Delivery)
{
	std::map<PeerId, Link>::iterator link = links_.find(peer);
	if (link != links_.end())
		link->second.other->push(NetEvent::Received, link->second.remoteId, data);
}

void LoopbackTransport::broadcast(const std::vector<uint8_t>& data, Delivery delivery, PeerId exceptPeer)
{
	for (std::map<PeerId, Link>::iterator link = links_.begin(); link != links_.end(); ++link)
		if (link->first != exceptPeer)
			send(link->first, data, delivery);
}

void LoopbackTransport::disconnect(PeerId peer)
{
	std::map<PeerId, Link>::iterator link = links_.find(peer);
	if (link == links_.end())
		return;
	Link ended = link->second;
	links_.erase(link);
	ended.other->links_.erase(ended.remoteId);
	ended.other->push(NetEvent::Disconnected, ended.remoteId, std::vector<uint8_t>());
	push(NetEvent::Disconnected, peer, std::vector<uint8_t>());
}

bool LoopbackTransport::poll(NetEvent& event)
{
	if (events_.empty())
		return false;
	event = events_.front();
	events_.pop_front();
	return true;
}

void LoopbackTransport::shutdown()
{
	while (!links_.empty())
		disconnect(links_.begin()->first);
	if (listening_)
		network_.listeners_.erase(port_);
	listening_ = false;
}

PeerId LoopbackTransport::addLink(LoopbackTransport* other, PeerId remoteId)
{
	PeerId id = nextPeer_++;
	links_[id] = Link{ other, remoteId };
	return id;
}

void LoopbackTransport::push(NetEvent::Kind kind, PeerId peer, const std::vector<uint8_t>& data)
{
	NetEvent event;
	event.kind = kind;
	event.peer = peer;
	event.data = data;
	events_.push_back(event);
}
