#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <irrlicht.h>

class irrBulletWorld;

class DebugOverlay
{
public:
	DebugOverlay(irr::IrrlichtDevice* device, irrBulletWorld* world) : device_(device), world_(world) {}
	void draw(const irr::core::vector3df& nodePosition);

private:
	irr::IrrlichtDevice* device_;
	irrBulletWorld* world_;
};

#endif
