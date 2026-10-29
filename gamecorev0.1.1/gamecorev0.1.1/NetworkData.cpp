/*
//////////////////////////////////////////////////////////////////////////
NETWORKDATA.CPP
stores and removes the remote players the server tells us about
By: Michael Carey
//////////////////////////////////////////////////////////////////////////
*/

#include "NetworkData.h"
#include <cstdlib>

NetworkData::NetworkData(irr::IrrlichtDevice* device, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver,
						 gamecore::SoundEngine* engine, PhysicsWorld* physics)
	: device_(device), smgr_(smgr), driver_(driver), engine_(engine), physics_(physics)
{
}

void NetworkData::add(uint16_t id, const std::string& name, uint8_t model)
{
	if (players_.count(id))
		return;
	if (model >= common_paths().model_paths.size())
		model = 0;
	players_.insert(std::make_pair(id, remotePlayer(device_, smgr_, driver_, engine_, physics_, model)));
	remotePlayer& added = players_[id];
	added.setId(id);
	added.setName(name);
}

void NetworkData::remove(uint16_t id)
{
	std::map<uint16_t, remotePlayer>::iterator player = players_.find(id);
	if (player == players_.end())
		return;
	player->second.delete_player();
	players_.erase(player);
}

void NetworkData::place(uint16_t id, const protocol::State& state, float now)
{
	std::map<uint16_t, remotePlayer>::iterator player = players_.find(id);
	if (player == players_.end())
		return;
	player->second.addSnapshot(state.seq, irr::core::vector3df(state.pos[0], state.pos[1], state.pos[2]),
		irr::core::vector3df(state.rot[0], state.rot[1], state.rot[2]), now);
	player->second.setState(state.state);
}

void NetworkData::update(float now)
{
	for (std::map<uint16_t, remotePlayer>::iterator player = players_.begin(); player != players_.end(); ++player)
		player->second.follow(now);
}

void NetworkData::hide(uint16_t id)
{
	std::map<uint16_t, remotePlayer>::iterator player = players_.find(id);
	if (player != players_.end())
		player->second.hide();
}

void NetworkData::show(uint16_t id)
{
	std::map<uint16_t, remotePlayer>::iterator player = players_.find(id);
	if (player != players_.end())
		player->second.show();
}

remotePlayer* NetworkData::findByNode(const std::string& nodeName)
{
	const std::string prefix = TYPE_PLAYER;
	if (nodeName.compare(0, prefix.size(), prefix) != 0)
		return 0;
	std::map<uint16_t, remotePlayer>::iterator player = players_.find((uint16_t)std::atoi(nodeName.c_str() + prefix.size()));
	return player == players_.end() ? 0 : &player->second;
}

std::vector<uint16_t> NetworkData::takeDamaged()
{
	std::vector<uint16_t> damaged;
	for (std::map<uint16_t, remotePlayer>::iterator player = players_.begin(); player != players_.end(); ++player)
		if (player->second.isBeingDamaged())
		{
			player->second.makeHealthy();
			damaged.push_back(player->first);
		}
	return damaged;
}
