#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <vector3d.h>
#include <irrString.h>
#include "Canvas.h"

namespace irr { class IrrlichtDevice; }

class PhysicsWorld;

class DebugOverlay
{
public:
	DebugOverlay(irr::IrrlichtDevice* device, Canvas& canvas, PhysicsWorld* physics) : device_(device), canvas_(canvas), physics_(physics) {}
	void draw(const irr::core::vector3df& nodePosition, const irr::core::stringw& playerDirection = irr::core::stringw(), const irr::core::stringw& cameraDirection = irr::core::stringw());

private:
	irr::IrrlichtDevice* device_;
	Canvas& canvas_;
	PhysicsWorld* physics_;
};

#endif
