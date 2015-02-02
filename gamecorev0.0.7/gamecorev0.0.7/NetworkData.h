#ifndef NETWORKDATA_H
#define NETWORKDATA_H

/*
//////////////////////////////////////////////////////////////////////////
NETWORKDATA.H
used to transport data from the client to the host
By: Michael Carey
//////////////////////////////////////////////////////////////////////////
*/
#define  MAX_PLAYERS 8

#include <sstream>
#include <string>
#include "player.h"
#include "remotePlayer.h"
#include "playerData.h"

class NetworkData
{
public:
	NetworkData();
	virtual ~NetworkData();
	const char* getNetworkData()  const;// get the remote data
	int length();
	void setString(player& thereInput);
	void setRemote(const char* ourData);
	void getRemote(remotePlayer& worldRemote, int dataIndex); //get the players remote data 
	int getMaxPlayers();//return the max number of players for our network game

	void setChatMessage(const std::string chat);
	const std::string getChatMessage();
	const std::string getThereChatMessage();
	int chatLength();


private:
	int dataSize_;
	int currentPlayers_;
	int maxPlayers_;
	std::ostringstream ourNetworkData;
	std::string ourNetworkChatMine;
	std::string ourNetworkChatTheres;
	playerData ourPlayerData[MAX_PLAYERS];
};

#endif