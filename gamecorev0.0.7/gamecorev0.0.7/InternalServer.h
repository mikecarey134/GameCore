#pragma once
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"
#include "player.h"
#include "GUI.h"

#define SERVER_PORT "1080"
#define CLIENT_PORT "1000"
#define ADDRESS "108.15.178.29"
#define MAX_MESSAGE_SIZE 2080

class InternalServer
{
public:
	InternalServer(bool online);
	~InternalServer(void);
	void messageLoop(player& thePlayer, GUI& GUI,irr::gui::IGUIFont* font);
	bool emptyMess(char* message);
	void shutdown();
	bool isOnline(){return online_;}
	
	
private:
	bool online_;
	//need to have an array of clients to send updates
	//std::vector<FPlayer> clients_;
	RakNet::RakNetStatistics* rss_;
	RakNet::RakPeerInterface* client_;
	// Holds packets
	RakNet::Packet* p;
	unsigned char packetIdentifier;
	bool isServer;
	char message[MAX_MESSAGE_SIZE];
};
unsigned char GetPacketIdentifier(RakNet::Packet *p);