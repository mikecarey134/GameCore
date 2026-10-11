#include "GameMap.h"
#include "GrassNode.h"
#include "ClueObject.h"
#include "NPC.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

GameMap::GameMap(IrrlichtDevice* device, IVideoDriver* driver, ISceneManager* smgr, irrBulletWorld* world)
	: mapLoader(device, driver, smgr, world), clue_(0), npc_(0)
{
}

void GameMap::onMapLoaded()
{
	array<ISceneNode*> nodes;
	smgr_->getSceneNodesFromType(ESNT_ANY, nodes);

	ISceneNode* landscape = smgr_->getSceneNodeFromName("static_landscape");
	if (landscape && landscape->getType() == ESNT_MESH)
	{
		aabbox3df box = landscape->getTransformedBoundingBox();
		ITriangleSelector* ground = smgr_->createOctreeTriangleSelector(static_cast<IMeshSceneNode*>(landscape)->getMesh(), landscape);
		for (u32 i = 0; i < nodes.size(); ++i)
		{
			if (stringc(nodes[i]->getName()).subString(0, 11) != "grass_area_")
				continue;
			vector3df centre = nodes[i]->getAbsolutePosition();
			vector3df size = nodes[i]->getScale();
			for(int g = 0; g < 100;++g){
				f32 x = centre.X + size.X * (rand() / (f32)RAND_MAX - 0.5f);
				f32 z = centre.Z + size.Z * (rand() / (f32)RAND_MAX - 0.5f);
				vector3df onGround;
				triangle3df hitTriangle;
				ISceneNode* hitNode = 0;
				if (!smgr_->getSceneCollisionManager()->getCollisionPoint(line3df(x, box.MaxEdge.Y + 1, z, x, box.MinEdge.Y - 1, z), ground, onGround, hitTriangle, hitNode))
					continue;
				grass_.push_back(GrassNode(device_,vector3df(0,0,0)));
				grass_.back().set_position(onGround);
			}
		}
		ground->drop();
	}

	for (u32 i = 0; i < nodes.size(); ++i)
		if (stringc(nodes[i]->getName()).subString(0, 5) == "clue_")
			clueLocations_.push_back(nodes[i]->getAbsolutePosition());
	for (u32 i = 0; i < nodes.size(); ++i)
		if (stringc(nodes[i]->getName()).subString(0, 4) == "npc_")
			npcLocations_.push_back(nodes[i]->getAbsolutePosition());
	if (clue_)
		clue_->placeOnMap();
}

void GameMap::onMapClosed()
{
	clueLocations_.clear();
	npcLocations_.clear();
	grass_.clear();
	if (npc_ && !npc_->isDead())
		npc_->despawn();
	if (clue_)
		clue_->getModel()->setVisible(false);
}
