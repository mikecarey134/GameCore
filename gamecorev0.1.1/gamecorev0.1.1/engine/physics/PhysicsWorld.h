#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include <cstdint>
#include <vector3d.h>
#include "CharacterController.h"

namespace irr
{
	class IrrlichtDevice;
	namespace scene { class ISceneNode; class IMeshSceneNode; }
	namespace video { class IVideoDriver; }
}

typedef uint32_t BodyId;
const BodyId NO_BODY = 0;

struct RayHit
{
	irr::core::vector3df point;
	irr::core::vector3df normal;
	BodyId body;
};

struct PhysicsStats
{
	int staticBodies;
	int dynamicBodies;
	int characters;
};

class PhysicsWorld
{
public:
	static PhysicsWorld* create(irr::IrrlichtDevice* device);

	virtual ~PhysicsWorld() {}

	virtual void setGravity(float y) = 0;
	virtual void step(float seconds) = 0;
	virtual BodyId addStaticMesh(irr::scene::IMeshSceneNode* node) = 0;
	virtual BodyId addDynamicBox(irr::scene::ISceneNode* node, float mass) = 0;
	virtual void removeBody(BodyId body) = 0;
	virtual CharacterController* addCharacter(const CharacterSettings& settings) = 0;
	virtual bool raycast(const irr::core::vector3df& from, const irr::core::vector3df& to, RayHit& hit) = 0;
	virtual void drawDebug(irr::video::IVideoDriver* driver) = 0;
	virtual PhysicsStats stats() const = 0;
};

#endif
