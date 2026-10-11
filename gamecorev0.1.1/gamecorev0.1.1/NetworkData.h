#ifndef NETWORKDATA_H
#define NETWORKDATA_H

/*
//////////////////////////////////////////////////////////////////////////
NETWORKDATA.H
stores and removes the remote players the server tells us about
By: Michael Carey
//////////////////////////////////////////////////////////////////////////
*/

#include <map>
#include <string>
#include <vector>
#include <irrlicht.h>
#include "remotePlayer.h"
#include "protocol/GameMessages.h"

class NetworkData
{
public:
	NetworkData(irr::IrrlichtDevice* device, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver,
		gamecore::SoundEngine* engine, irrBulletWorld* world);

	void add(uint16_t id, const std::string& name, uint8_t model);
	void remove(uint16_t id);
	void place(uint16_t id, const protocol::State& state, float now);
	void update(float now);
	void hide(uint16_t id);
	void show(uint16_t id);
	remotePlayer* findByNode(const std::string& nodeName);
	std::vector<uint16_t> takeDamaged();

	std::map<uint16_t, remotePlayer>& getRemotePlayers() { return players_; }

private:
	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	gamecore::SoundEngine* engine_;
	irrBulletWorld* world_;
	std::map<uint16_t, remotePlayer> players_;
};

#endif
