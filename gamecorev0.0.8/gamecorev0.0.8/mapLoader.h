#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <irrlicht.h>
#include <irrbullet.h>
#include <vector>

#define PREFIX "map/"

class mapLoader
{
public:
	mapLoader (irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr, irrBulletWorld* world);

	~mapLoader() {}

	void setMap (std::string mapName) { mapName_ = PREFIX; mapName_ += mapName.c_str(); }

	void loadMap();
	void closeMap();

	const std::vector<irr::core::vector3df> getClueLocations() const { return clueLocations_; }

private:
	irr::IrrlichtDevice* device_;
	irr::video::IVideoDriver* driver_;
	irr::scene::ISceneManager* smgr_;
	irrBulletWorld* world_;

	irr::core::string<char> mapName_;
	std::vector<irr::core::vector3df> clueLocations_;
};

#endif