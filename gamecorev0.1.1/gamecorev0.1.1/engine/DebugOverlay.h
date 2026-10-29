#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <vector3d.h>
#include "Canvas.h"

namespace irr { class IrrlichtDevice; }

class PhysicsWorld;

class DebugOverlay
{
public:
	DebugOverlay(irr::IrrlichtDevice* device, Canvas& canvas, PhysicsWorld* physics) : device_(device), canvas_(canvas), physics_(physics) {}
	void draw(const irr::core::vector3df& nodePosition);

private:
	irr::IrrlichtDevice* device_;
	Canvas& canvas_;
	PhysicsWorld* physics_;
};

#endif
