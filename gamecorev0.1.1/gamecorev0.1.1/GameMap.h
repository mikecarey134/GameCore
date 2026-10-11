#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "mapLoader.h"

#define PREFIX "map/"

class GameMap : public mapLoader
{
public:
	GameMap(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr, irrBulletWorld* world);

	const std::vector<irr::core::vector3df> getClueLocations() const { return clueLocations_; }

protected:
	void onMapLoaded() override;
	void onMapClosed() override;

private:
	std::vector<irr::core::vector3df> clueLocations_;
};

#endif
