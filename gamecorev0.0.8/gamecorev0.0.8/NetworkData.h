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
#include <vector>

class NetworkData
{
public:
	NetworkData(irr::IrrlichtDevice* device,char* filename,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		irrklang::ISoundEngine* engine, irrBulletWorld* world);

	virtual ~NetworkData();
	const char* getNetworkData()  const;// get the remote data
	int length();
	void setString(player& thereInput);
	void setRemote(const char* ourData);
	void getRemote(remotePlayer& worldRemote, int dataIndex); //get the players remote data 
	int getMaxPlayers();//return the max number of players for our network game

	void setChatMessage(char* chat);
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
	//playerData ourPlayerData[MAX_PLAYERS];
	std::map<std::string,remotePlayer> players_;
	
	irr::IrrlichtDevice* device_;
	char* filename_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	irrklang::ISoundEngine* engine_;
	irrBulletWorld* world_;

};

#endif