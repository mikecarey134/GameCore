#include "EnetTransport.h"
#include <enet/enet.h>

EnetTransport::EnetTransport(uint32_t timeoutMs) : host_(0), nextPeer_(1), timeoutMs_(timeoutMs)
{
	enet_initialize();
}

EnetTransport::~EnetTransport()
{
	shutdown();
	enet_deinitialize();
}

bool EnetTransport::listen(uint16_t port, int maxPeers)
{
	if (host_)
		return false;
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;
	host_ = enet_host_create(&address, (size_t)maxPeers, 2, 0, 0);
	return host_ != 0;
}

bool EnetTransport::connect(const std::string& host, uint16_t port)
{
	if (host_)
		return false;
	ENetAddress address;
	if (enet_address_set_host(&address, host.c_str()) != 0)
		return false;
	address.port = port;
	host_ = enet_host_create(0, 1, 2, 0, 0);
	if (!host_)
		return false;
	ENetPeer* peer = enet_host_connect(host_, &address, 2, 0);
	if (!peer)
	{
		enet_host_destroy(host_);
		host_ = 0;
		return false;
	}
	track(peer);
	return true;
}

void EnetTransport::send(PeerId peer, const std::vector<uint8_t>& data, Delivery delivery)
{
	std::map<PeerId, ENetPeer*>::iterator found = peers_.find(peer);
	if (found == peers_.end())
		return;
	bool reliable = delivery == Reliable;
	ENetPacket* packet = enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	if (enet_peer_send(found->second, reliable ? 0 : 1, packet) != 0)
		enet_packet_destroy(packet);
	else
		traffic_[peer].bytesSent += data.size();
}

void EnetTransport::broadcast(const std::vector<uint8_t>& data, Delivery delivery, PeerId exceptPeer)
{
	for (std::map<PeerId, ENetPeer*>::iterator peer = peers_.begin(); peer != peers_.end(); ++peer)
		if (peer->first != exceptPeer)
			send(peer->first, data, delivery);
}

void EnetTransport::disconnect(PeerId peer)
{
	std::map<PeerId, ENetPeer*>::iterator found = peers_.find(peer);
	if (found != peers_.end())
		enet_peer_disconnect_later(found->second, 0);
}

bool EnetTransport::poll(NetEvent& event)
{
	if (!host_)
		return false;
	ENetEvent happened;
	while (enet_host_service(host_, &happened, 0) > 0)
	{
		PeerId id = (PeerId)(uintptr_t)happened.peer->data;
		switch (happened.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			event.kind = NetEvent::Connected;
			event.peer = id != NO_PEER ? id : track(happened.peer);
			event.data.clear();
			return true;
		case ENET_EVENT_TYPE_RECEIVE:
			event.kind = NetEvent::Received;
			event.peer = id;
			event.data.assign(happened.packet->data, happened.packet->data + happened.packet->dataLength);
			enet_packet_destroy(happened.packet);
			if (id == NO_PEER)
				continue;
			traffic_[id].bytesReceived += event.data.size();
			return true;
		case ENET_EVENT_TYPE_DISCONNECT:
			happened.peer->data = 0;
			peers_.erase(id);
			traffic_.erase(id);
			if (id == NO_PEER)
				continue;
			event.kind = NetEvent::Disconnected;
			event.peer = id;
			event.data.clear();
			return true;
		default:
			break;
		}
	}
	return false;
}

void EnetTransport::shutdown()
{
	if (!host_)
		return;
	for (std::map<PeerId, ENetPeer*>::iterator peer = peers_.begin(); peer != peers_.end(); ++peer)
		enet_peer_disconnect_now(peer->second, 0);
	peers_.clear();
	traffic_.clear();
	enet_host_destroy(host_);
	host_ = 0;
}

std::string EnetTransport::address(PeerId peer) const
{
	std::map<PeerId, ENetPeer*>::const_iterator found = peers_.find(peer);
	char text[64] = "";
	if (found == peers_.end() || enet_address_get_host_ip(&found->second->address, text, sizeof(text)) != 0)
		return "";
	return text;
}

PeerStats EnetTransport::stats(PeerId peer) const
{
	PeerStats result = PeerStats{ 0, 0, 0, 0.0f };
	std::map<PeerId, PeerStats>::const_iterator counted = traffic_.find(peer);
	if (counted != traffic_.end())
		result = counted->second;
	std::map<PeerId, ENetPeer*>::const_iterator found = peers_.find(peer);
	if (found != peers_.end())
	{
		result.roundTripMs = found->second->roundTripTime;
		result.packetLoss = (float)found->second->packetLoss / ENET_PEER_PACKET_LOSS_SCALE;
	}
	return result;
}

PeerId EnetTransport::track(ENetPeer* peer)
{
	PeerId id = nextPeer_++;
	peer->data = (void*)(uintptr_t)id;
	enet_peer_timeout(peer, 0, timeoutMs_, timeoutMs_);
	peers_[id] = peer;
	traffic_[id] = PeerStats{ 0, 0, 0, 0.0f };
	return id;
}
