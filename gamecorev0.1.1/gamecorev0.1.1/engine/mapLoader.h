#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <irrlicht.h>
#include <irrbullet.h>
#include <string>
#include <vector>

class mapLoader
{
public:
	mapLoader (irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr, irrBulletWorld* world);

	virtual ~mapLoader() {}

	void setMap (std::string mapPath) { mapName_ = mapPath.c_str(); }

	void loadMap();
	void closeMap();

	std::vector<irr::core::vector3df> getSpawnPoints() const;

protected:
	virtual void onMapLoaded() {}
	virtual void onMapClosed() {}

	irr::IrrlichtDevice* device_;
	irr::video::IVideoDriver* driver_;
	irr::scene::ISceneManager* smgr_;
	irrBulletWorld* world_;

private:
	irr::core::string<char> mapName_;
	std::vector<ICollisionObject*> bodies_;
	std::vector<irr::scene::ISceneNode*> mapNodes_;
	std::vector<irr::core::vector3df> spawnPoints_;
};

#endif