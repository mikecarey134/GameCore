#ifndef NETWORKDATA_H
#define NETWORKDATA_H

/*

NETWORKDATA.H

The Network Data class

*/

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
	const char* getNetworkData()  const;
	int length();
	void setString(player& thereInput);
	void setRemote(const char* ourData);
	void getRemote(remotePlayer& worldRemote, int dataIndex); 
	int getMaxPlayers();

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
	playerData ourPlayerData[9];
};

#endif