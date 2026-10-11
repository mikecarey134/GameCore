#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <irrlicht.h>

class PhysicsWorld;

class DebugOverlay
{
public:
	DebugOverlay(irr::IrrlichtDevice* device, PhysicsWorld* physics) : device_(device), physics_(physics) {}
	void draw(const irr::core::vector3df& nodePosition);

private:
	irr::IrrlichtDevice* device_;
	PhysicsWorld* physics_;
};

#endif
