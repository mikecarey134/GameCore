#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "mapLoader.h"
#include "GrassNode.h"

class ClueObject;
class NPC;

#define PREFIX "map/"

class GameMap : public mapLoader
{
public:
	GameMap(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr, irrBulletWorld* world);

	const std::vector<irr::core::vector3df> getClueLocations() const { return clueLocations_; }
	void setClue(ClueObject* clue) { clue_ = clue; }
	void setNpc(NPC* npc) { npc_ = npc; }
	const std::vector<irr::core::vector3df> getNpcLocations() const { return npcLocations_; }

protected:
	void onMapLoaded() override;
	void onMapClosed() override;

private:
	std::vector<irr::core::vector3df> clueLocations_;
	ClueObject* clue_;
	NPC* npc_;
	std::vector<irr::core::vector3df> npcLocations_;
	std::vector<GrassNode> grass_;
};

#endif
