#include "PhysicsWorld.h"
#include <irrlicht.h>
#include <irrbullet.h>
#include <map>
#include "kinematiccharactercontroller.h"

using namespace irr;
using namespace core;

namespace
{
	const float FIXED_STEP = 1.0f / 60.0f;
	const u32 MAX_SUBSTEPS = 8;

	class BulletCharacter : public CharacterController
	{
	public:
		BulletCharacter(irrBulletWorld* world, const CharacterSettings& settings, int& count)
			: controller_(world, settings.radius, settings.height, settings.stepHeight, settings.collidesWithCharacters), count_(count)
		{
			controller_.setMaxSlope(settings.maxSlope);
			controller_.setGravity(settings.gravity);
			controller_.setJumpForce(settings.jumpSpeed);
			++count_;
		}

		~BulletCharacter() override { --count_; }

		void warp(const vector3df& position) override { controller_.warp(position); }
		vector3df position() const override { return controller_.getWorldTransform().getTranslation(); }
		void setWalk(const vector3df& unitsPerSecond) override { controller_.setPositionIncrementPerSimulatorStep(unitsPerSecond * FIXED_STEP); }
		void jump() override { controller_.jump(); }
		bool onGround() const override { return controller_.isOnGround(); }
		void setGravity(float gravity) override { controller_.setGravity(gravity); }
		void setJumpSpeed(float speed) override { controller_.setJumpForce(speed); }
		void setMaxSlope(float radians) override { controller_.setMaxSlope(radians); }

	private:
		IKinematicCharacterController controller_;
		int& count_;
	};

	class BulletPhysicsWorld : public PhysicsWorld
	{
	public:
		explicit BulletPhysicsWorld(IrrlichtDevice* device)
			: world_(createIrrBulletWorld(device, true, true)), nextBody_(1), characters_(0)
		{
			world_->setDebugMode(EPDM_DrawAabb | EPDM_DrawContactPoints);
		}

		~BulletPhysicsWorld() override
		{
			delete world_;
		}

		void setGravity(float y) override
		{
			world_->setGravity(vector3df(0.0f, y, 0.0f));
		}

		void step(float seconds) override
		{
			world_->stepSimulation(seconds, MAX_SUBSTEPS, FIXED_STEP);
		}

		BodyId addStaticMesh(scene::IMeshSceneNode* node) override
		{
			return add(world_->addRigidBody(new IBvhTriangleMeshShape(node, node->getMesh(), 0.0f)), statics_);
		}

		BodyId addDynamicBox(scene::ISceneNode* node, float mass) override
		{
			return add(world_->addRigidBody(new IBoxShape(node, mass)), dynamics_);
		}

		void removeBody(BodyId body) override
		{
			if (!remove(body, statics_))
				remove(body, dynamics_);
		}

		CharacterController* addCharacter(const CharacterSettings& settings) override
		{
			return new BulletCharacter(world_, settings, characters_);
		}

		bool raycast(const vector3df&, const vector3df&, RayHit&) override
		{
			return false;
		}

		void drawDebug(video::IVideoDriver*) override
		{
			world_->debugDrawWorld(true);
		}

		PhysicsStats stats() const override
		{
			return PhysicsStats{ (int)statics_.size(), (int)dynamics_.size(), characters_ };
		}

	private:
		BodyId add(IRigidBody* body, std::map<BodyId, IRigidBody*>& bodies)
		{
			BodyId id = nextBody_++;
			bodies[id] = body;
			return id;
		}

		bool remove(BodyId id, std::map<BodyId, IRigidBody*>& bodies)
		{
			std::map<BodyId, IRigidBody*>::iterator body = bodies.find(id);
			if (body == bodies.end())
				return false;
			body->second->includeNodeOnRemoval(false);
			world_->removeCollisionObject(body->second);
			bodies.erase(body);
			return true;
		}

		irrBulletWorld* world_;
		std::map<BodyId, IRigidBody*> statics_;
		std::map<BodyId, IRigidBody*> dynamics_;
		BodyId nextBody_;
		int characters_;
	};
}

PhysicsWorld* PhysicsWorld::create(IrrlichtDevice* device)
{
	return new BulletPhysicsWorld(device);
}
