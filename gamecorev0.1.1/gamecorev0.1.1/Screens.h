#ifndef SCREENS_H
#define SCREENS_H

#include <irrlicht.h>

class player;

class Hud
{
public:
	explicit Hud(irr::IrrlichtDevice* device) : device_(device) {}
	void draw(const player& thePlayer);

private:
	irr::IrrlichtDevice* device_;
};

#endif
