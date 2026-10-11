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
	if (smgr_->getSceneNodeFromName("static_landscape"))
	{
		for(int g = 0; g < 100;++g){
			GrassNode* grass_node = new GrassNode(device_,vector3df(0,0,0));
			grass_node->set_position(vector3df((f32)(rand()%200) ,(f32)(rand() % 1 + (-9)), (f32)(rand()% 200)));
		}
	}

	clueLocations_.push_back(vector3df(167, -4, 67));
	clueLocations_.push_back(vector3df(114, -4, 80));

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
