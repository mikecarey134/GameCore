//////////////////////////////////////////////////////////////////////////
/*
	InternalServer.h
	By : Michael Carey
	Purpose : provide communication to game Server from the client side
	in the forms of chat and player data
*/
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <chrono>
#include "player.h"
#include "NetworkData.h"
#include "remotePlayer.h"
#include "ChatQueue.h"
#include "client/NetClient.h"
#include "net/EnetTransport.h"

class InternalServer
{
public:
	InternalServer(bool online,irr::IrrlichtDevice* device,const char* filename,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		gamecore::SoundEngine* engine, PhysicsWorld* physics, char* ip
        ,int remote_port, int local_port,ChatQueue* message_buffer, player* thePlayer);

	~InternalServer(void);
	void messageLoop(player& thePlayer);
	bool emptyMess(const char* message);
	void shutdown(player& thePlayer);
	bool isOnline(){return client_.isOnline();}

	NetworkData* getNetworkData() { return &networkData_; }

private:
	NetClient::Events events();
	float seconds() const;

	EnetTransport transport_;
	NetworkData networkData_;
	ChatQueue* message_buffer_;
	player* player_;
	std::chrono::steady_clock::time_point started_;
	NetClient client_;
};
