#include "PhysicsWorld.h"
#include <irrlicht.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>

using namespace irr;

namespace
{
	const float FIXED_STEP = 1.0f / 60.0f;
	const int MAX_STEPS = 8;
	const JPH::ObjectLayer STATIC_LAYER = 0;
	const JPH::ObjectLayer MOVING_LAYER = 1;

	JPH::Vec3 toJolt(const core::vector3df& v) { return JPH::Vec3(v.X, v.Y, v.Z); }
	core::vector3df toIrr(JPH::Vec3Arg v) { return core::vector3df(v.GetX(), v.GetY(), v.GetZ()); }

	JPH::Quat rotationOf(scene::ISceneNode* node)
	{
		core::matrix4 m;
		m.setRotationDegrees(node->getRotation());
		const f32* p = m.pointer();
		JPH::Mat44 rotation(JPH::Vec4(p[0], p[1], p[2], 0.0f), JPH::Vec4(p[4], p[5], p[6], 0.0f),
			JPH::Vec4(p[8], p[9], p[10], 0.0f), JPH::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		return rotation.GetQuaternion().Normalized();
	}

	core::vector3df degreesOf(JPH::QuatArg rotation)
	{
		JPH::Mat44 r = JPH::Mat44::sRotation(rotation);
		core::matrix4 m;
		for (int column = 0; column < 3; ++column)
			for (int row = 0; row < 3; ++row)
				m[column * 4 + row] = r(row, column);
		return m.getRotationDegrees();
	}

	void registerJolt()
	{
		static bool registered = false;
		if (registered)
			return;
		JPH::RegisterDefaultAllocator();
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();
		registered = true;
	}

	class LineRenderer : public JPH::DebugRendererSimple
	{
	public:
		video::IVideoDriver* driver = nullptr;

		void DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color) override
		{
			driver->draw3DLine(toIrr(from), toIrr(to), video::SColor(255, color.r, color.g, color.b));
		}

		void DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float) override {}
	};

	class SolidCharacters : public JPH::CharacterContactListener
	{
	public:
		void OnCharacterContactAdded(const JPH::CharacterVirtual*, const JPH::CharacterContact&, JPH::CharacterContactSettings& settings) override
		{
			settings.mCanPushCharacter = false;
		}

		void OnCharacterContactPersisted(const JPH::CharacterVirtual*, const JPH::CharacterContact&, JPH::CharacterContactSettings& settings) override
		{
			settings.mCanPushCharacter = false;
		}
	};

	SolidCharacters solidCharacters;

	class JoltPhysicsWorld;

	class JoltCharacter : public CharacterController
	{
	public:
		JoltCharacter(JoltPhysicsWorld* world, const CharacterSettings& settings);
		~JoltCharacter() override;

		void warp(const core::vector3df& position) override { character_->SetPosition(toJolt(position)); }
		core::vector3df position() const override { return toIrr(character_->GetPosition()); }
		void setWalk(const core::vector3df& unitsPerSecond) override { walk_ = toJolt(unitsPerSecond); }
		void jump() override { jump_ = true; }
		bool onGround() const override { return character_->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround; }
		void setGravity(float gravity) override { gravity_ = gravity; }
		void setJumpSpeed(float speed) override { jumpSpeed_ = speed; }
		void setMaxSlope(float radians) override { character_->SetMaxSlopeAngle(radians); }

		void update(float seconds);
		void detach() { world_ = nullptr; }

	private:
		JoltPhysicsWorld* world_;
		JPH::Ref<JPH::CharacterVirtual> character_;
		JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings_;
		JPH::Vec3 walk_ = JPH::Vec3::sZero();
		float gravity_;
		float jumpSpeed_;
		bool jump_ = false;
		bool collidesWithCharacters_;
	};

	class JoltPhysicsWorld : public PhysicsWorld
	{
	public:
		JoltPhysicsWorld()
			: broadPhaseLayers_(2, 2), layerPairs_(2),
			  tempAllocator_(16 * 1024 * 1024), jobSystem_(JPH::cMaxPhysicsJobs),
			  accumulator_(0.0f), nextBody_(1)
		{
			broadPhaseLayers_.MapObjectToBroadPhaseLayer(STATIC_LAYER, JPH::BroadPhaseLayer(0));
			broadPhaseLayers_.MapObjectToBroadPhaseLayer(MOVING_LAYER, JPH::BroadPhaseLayer(1));
			layerPairs_.EnableCollision(STATIC_LAYER, MOVING_LAYER);
			layerPairs_.EnableCollision(MOVING_LAYER, MOVING_LAYER);
			objectVsBroadPhase_.reset(new JPH::ObjectVsBroadPhaseLayerFilterTable(broadPhaseLayers_, 2, layerPairs_, 2));
			system_.Init(65536, 0, 65536, 10240, broadPhaseLayers_, *objectVsBroadPhase_, layerPairs_);
		}

		~JoltPhysicsWorld() override
		{
			for (JoltCharacter* character : characters_)
				character->detach();
			for (auto& body : bodies_)
			{
				bodies().RemoveBody(body.second.id);
				bodies().DestroyBody(body.second.id);
			}
		}

		void setGravity(float y) override
		{
			system_.SetGravity(JPH::Vec3(0.0f, y, 0.0f));
		}

		void step(float seconds) override
		{
			accumulator_ += seconds;
			int steps = 0;
			while (accumulator_ >= FIXED_STEP && steps < MAX_STEPS)
			{
				for (JoltCharacter* character : characters_)
					character->update(FIXED_STEP);
				system_.Update(FIXED_STEP, 1, &tempAllocator_, &jobSystem_);
				accumulator_ -= FIXED_STEP;
				++steps;
			}
			if (steps == MAX_STEPS)
				accumulator_ = 0.0f;
			syncNodes();
		}

		BodyId addStaticMesh(scene::IMeshSceneNode* node) override
		{
			JPH::TriangleList triangles;
			scene::IMesh* mesh = node->getMesh();
			const core::matrix4& transform = node->getAbsoluteTransformation();
			for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b)
			{
				scene::IMeshBuffer* buffer = mesh->getMeshBuffer(b);
				for (u32 i = 0; i + 2 < buffer->getIndexCount(); i += 3)
				{
					core::vector3df corner[3];
					for (int c = 0; c < 3; ++c)
					{
						u32 index = buffer->getIndexType() == video::EIT_16BIT
							? buffer->getIndices()[i + c]
							: reinterpret_cast<const u32*>(buffer->getIndices())[i + c];
						corner[c] = buffer->getPosition(index);
						transform.transformVect(corner[c]);
					}
					triangles.push_back(JPH::Triangle(JPH::Float3(corner[0].X, corner[0].Y, corner[0].Z),
						JPH::Float3(corner[1].X, corner[1].Y, corner[1].Z), JPH::Float3(corner[2].X, corner[2].Y, corner[2].Z)));
				}
			}
			JPH::MeshShapeSettings settings(triangles);
			JPH::ShapeSettings::ShapeResult shape = settings.Create();
			if (shape.HasError())
				return NO_BODY;
			JPH::BodyCreationSettings body(shape.Get(), JPH::RVec3::sZero(), JPH::Quat::sIdentity(), JPH::EMotionType::Static, STATIC_LAYER);
			return add(bodies().CreateAndAddBody(body, JPH::EActivation::DontActivate), nullptr);
		}

		BodyId addDynamicBox(scene::ISceneNode* node, float mass) override
		{
			core::vector3df half = node->getBoundingBox().getExtent() * node->getScale() * 0.5f;
			float smallest = std::min(half.X, std::min(half.Y, half.Z));
			JPH::Ref<JPH::Shape> shape = new JPH::BoxShape(toJolt(half), std::min(JPH::cDefaultConvexRadius, smallest * 0.5f));
			JPH::BodyCreationSettings body(shape, toJolt(node->getAbsolutePosition()), rotationOf(node), JPH::EMotionType::Dynamic, MOVING_LAYER);
			body.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
			body.mMassPropertiesOverride.mMass = mass;
			return add(bodies().CreateAndAddBody(body, JPH::EActivation::Activate), node);
		}

		void removeBody(BodyId id) override
		{
			auto body = bodies_.find(id);
			if (body == bodies_.end())
				return;
			bodies().RemoveBody(body->second.id);
			bodies().DestroyBody(body->second.id);
			bodies_.erase(body);
		}

		CharacterController* addCharacter(const CharacterSettings& settings) override
		{
			return new JoltCharacter(this, settings);
		}

		bool raycast(const core::vector3df& from, const core::vector3df& to, RayHit& hit) override
		{
			JPH::RRayCast ray(toJolt(from), toJolt(to - from));
			JPH::RayCastResult result;
			if (!system_.GetNarrowPhaseQuery().CastRay(ray, result))
				return false;
			JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
			hit.point = toIrr(point);
			hit.body = NO_BODY;
			for (auto& body : bodies_)
				if (body.second.id == result.mBodyID)
					hit.body = body.first;
			JPH::BodyLockRead lock(system_.GetBodyLockInterface(), result.mBodyID);
			hit.normal = lock.Succeeded() ? toIrr(lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point)) : core::vector3df();
			return true;
		}

		void drawDebug(video::IVideoDriver* driver) override
		{
			if (!renderer_)
				renderer_.reset(new LineRenderer());
			renderer_->driver = driver;
			video::SMaterial material;
			material.Lighting = false;
			driver->setMaterial(material);
			driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
			JPH::BodyManager::DrawSettings settings;
			settings.mDrawShape = false;
			settings.mDrawBoundingBox = true;
			system_.DrawBodies(settings, renderer_.get());
		}

		PhysicsStats stats() const override
		{
			int statics = 0;
			for (auto& body : bodies_)
				if (!body.second.node)
					++statics;
			return PhysicsStats{ statics, (int)bodies_.size() - statics, (int)characters_.size() };
		}

		JPH::PhysicsSystem& system() { return system_; }
		JPH::TempAllocator& tempAllocator() { return tempAllocator_; }
		JPH::CharacterVsCharacterCollisionSimple& characterCollision() { return characterCollision_; }
		JPH::ObjectVsBroadPhaseLayerFilter& objectVsBroadPhase() { return *objectVsBroadPhase_; }
		JPH::ObjectLayerPairFilter& layerPairs() { return layerPairs_; }
		void track(JoltCharacter* character) { characters_.push_back(character); }
		void forget(JoltCharacter* character) { characters_.erase(std::remove(characters_.begin(), characters_.end(), character), characters_.end()); }

	private:
		struct Body
		{
			JPH::BodyID id;
			scene::ISceneNode* node;
		};

		JPH::BodyInterface& bodies() { return system_.GetBodyInterfaceNoLock(); }

		BodyId add(JPH::BodyID id, scene::ISceneNode* node)
		{
			if (id.IsInvalid())
				return NO_BODY;
			BodyId body = nextBody_++;
			bodies_[body] = Body{ id, node };
			return body;
		}

		void syncNodes()
		{
			for (auto& body : bodies_)
			{
				if (!body.second.node)
					continue;
				JPH::RVec3 position;
				JPH::Quat rotation;
				bodies().GetPositionAndRotation(body.second.id, position, rotation);
				body.second.node->setPosition(toIrr(position));
				body.second.node->setRotation(degreesOf(rotation));
			}
		}

		JPH::BroadPhaseLayerInterfaceTable broadPhaseLayers_;
		JPH::ObjectLayerPairFilterTable layerPairs_;
		std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> objectVsBroadPhase_;
		JPH::PhysicsSystem system_;
		JPH::TempAllocatorImpl tempAllocator_;
		JPH::JobSystemSingleThreaded jobSystem_;
		JPH::CharacterVsCharacterCollisionSimple characterCollision_;
		std::map<BodyId, Body> bodies_;
		std::vector<JoltCharacter*> characters_;
		std::unique_ptr<LineRenderer> renderer_;
		float accumulator_;
		BodyId nextBody_;
	};

	JoltCharacter::JoltCharacter(JoltPhysicsWorld* world, const CharacterSettings& settings)
		: world_(world), gravity_(settings.gravity), jumpSpeed_(settings.jumpSpeed),
		  collidesWithCharacters_(settings.collidesWithCharacters)
	{
		JPH::CharacterVirtualSettings virtualSettings;
		virtualSettings.mShape = new JPH::CylinderShape(0.5f * settings.height + settings.radius, settings.radius);
		virtualSettings.mMaxSlopeAngle = settings.maxSlope;
		virtualSettings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), 0.5f * settings.height);
		character_ = new JPH::CharacterVirtual(&virtualSettings, JPH::RVec3::sZero(), JPH::Quat::sIdentity(), &world_->system());
		updateSettings_.mWalkStairsStepUp = JPH::Vec3(0.0f, settings.stepHeight, 0.0f);
		if (collidesWithCharacters_)
		{
			character_->SetCharacterVsCharacterCollision(&world_->characterCollision());
			character_->SetListener(&solidCharacters);
			world_->characterCollision().Add(character_);
		}
		world_->track(this);
	}

	JoltCharacter::~JoltCharacter()
	{
		if (!world_)
			return;
		if (collidesWithCharacters_)
			world_->characterCollision().Remove(character_);
		world_->forget(this);
	}

	void JoltCharacter::update(float seconds)
	{
		JPH::Vec3 up = JPH::Vec3::sAxisY();
		float vertical = character_->GetLinearVelocity().GetY();
		float groundVertical = character_->GetGroundVelocity().GetY();
		if (character_->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround && vertical - groundVertical < 0.1f)
			vertical = groundVertical + (jump_ ? jumpSpeed_ : 0.0f);
		jump_ = false;
		vertical -= gravity_ * seconds;
		character_->SetLinearVelocity(JPH::Vec3(walk_.GetX(), 0.0f, walk_.GetZ()) + vertical * up);
		character_->ExtendedUpdate(seconds, -gravity_ * up, updateSettings_,
			JPH::DefaultBroadPhaseLayerFilter(world_->objectVsBroadPhase(), MOVING_LAYER),
			JPH::DefaultObjectLayerFilter(world_->layerPairs(), MOVING_LAYER),
			JPH::BodyFilter(), JPH::ShapeFilter(), world_->tempAllocator());
	}
}

PhysicsWorld* PhysicsWorld::create(IrrlichtDevice*)
{
	registerJolt();
	return new JoltPhysicsWorld();
}
