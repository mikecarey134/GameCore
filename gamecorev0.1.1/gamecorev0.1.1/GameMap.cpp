#include "GameMap.h"
#include "GrassNode.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

GameMap::GameMap(IrrlichtDevice* device, IVideoDriver* driver, ISceneManager* smgr, irrBulletWorld* world)
	: mapLoader(device, driver, smgr, world)
{
}

void GameMap::onMapLoaded()
{
	ISceneNode* landscape = smgr_->getSceneNodeFromName("static_landscape");
	if (landscape && landscape->getType() == ESNT_MESH)
	{
		aabbox3df box = landscape->getTransformedBoundingBox();
		ITriangleSelector* ground = smgr_->createOctreeTriangleSelector(static_cast<IMeshSceneNode*>(landscape)->getMesh(), landscape);
		for(int g = 0; g < 100;++g){
			f32 x = box.MinEdge.X + (box.MaxEdge.X - box.MinEdge.X) * rand() / (f32)RAND_MAX;
			f32 z = box.MinEdge.Z + (box.MaxEdge.Z - box.MinEdge.Z) * rand() / (f32)RAND_MAX;
			vector3df onGround(x, box.MinEdge.Y, z);
			triangle3df hitTriangle;
			ISceneNode* hitNode = 0;
			smgr_->getSceneCollisionManager()->getCollisionPoint(line3df(x, box.MaxEdge.Y + 1, z, x, box.MinEdge.Y - 1, z), ground, onGround, hitTriangle, hitNode);
			GrassNode* grass_node = new GrassNode(device_,vector3df(0,0,0));
			grass_node->set_position(onGround);
		}
		ground->drop();
	}

	array<ISceneNode*> nodes;
	smgr_->getSceneNodesFromType(ESNT_ANY, nodes);
	for (u32 i = 0; i < nodes.size(); ++i)
		if (stringc(nodes[i]->getName()).subString(0, 5) == "clue_")
			clueLocations_.push_back(nodes[i]->getAbsolutePosition());

	//temp skybox later we will have one based off the time of day
	smgr_->addSkyBoxSceneNode(
		driver_->getTexture("map/sky/bstop.jpg"),
		driver_->getTexture("map/sky/bstop.jpg"),
		driver_->getTexture("map/sky/bsleft.jpg"),
		driver_->getTexture("map/sky/bsright.jpg"),
		driver_->getTexture("map/sky/bsfront.jpg"),
		driver_->getTexture("map/sky/bsback.jpg"));
}

void GameMap::onMapClosed()
{
	clueLocations_.clear();
}
