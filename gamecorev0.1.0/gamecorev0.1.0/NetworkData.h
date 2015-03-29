#ifndef NETWORKDATA_H
#define NETWORKDATA_H

/*
//////////////////////////////////////////////////////////////////////////
NETWORKDATA.H
used to transport data from the client to the host
as well as store and remove remote players 
By: Michael Carey
//////////////////////////////////////////////////////////////////////////
*/
#define  MAX_PLAYERS 8

#pragma once
#include "ChatQueue.h"
#include <sstream>
#include <string>
#include "player.h"
#include "remotePlayer.h"
#include <vector>
#include"common.h"
#include <iostream>
#include <irrlicht.h>

class NetworkData
{
public:
	//ctor
	NetworkData(irr::IrrlichtDevice* device,char* filename,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		irrklang::ISoundEngine* engine, irrBulletWorld* world, ChatQueue* message_buffer);
	//dtor
	virtual ~NetworkData();

	int chatLength ();
	int length     ();
	
	//setters
	void  setChatMessage  (char* chat);
	void  setString       (player& thereInput);
	void  setRemote       (const char* ourData);
	
	//getters
    const char*       getNetworkData      ()const;// get the remote data
	const std::string getChatMessage      ();
	const std::string getThereChatMessage ();
	int               getMaxPlayers       ();//return the max number of players for our network game


private:

	int dataSize_;
	int currentPlayers_;
	int maxPlayers_;
	std::ostringstream ourNetworkData;
	std::string ourNetworkChatMine;
	std::string ourNetworkChatTheres;
	std::map<std::string,remotePlayer> players_;
	irr::IrrlichtDevice* device_;
	char* filename_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	irrklang::ISoundEngine* engine_;
	irrBulletWorld* world_;
	ChatQueue* message_buffer_;
	common_paths paths_;

};

#endif