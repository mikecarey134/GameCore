#include "doctest.h"
#include "physics/PhysicsWorld.h"
#include <irrlicht.h>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <memory>
#include <vector>

using namespace irr;
using namespace core;

namespace
{
	const float FRAME = 1.0f / 60.0f;
	const float PI_F = 3.14159265f;

	CharacterSettings playerSettings()
	{
		CharacterSettings settings;
		settings.gravity = 150.0f;
		settings.jumpSpeed = 60.0f;
		return settings;
	}

	struct TestWorld
	{
		TestWorld() : device(createDevice(video::EDT_NULL)), physics(PhysicsWorld::create(device))
		{
			physics->setGravity(-20.0f);
		}

		~TestWorld()
		{
			characters.clear();
			delete physics;
			device->drop();
		}

		scene::IMeshSceneNode* plane(const vector3df& position, const vector3df& rotation = vector3df())
		{
			scene::IMesh* mesh = device->getSceneManager()->getGeometryCreator()->createPlaneMesh(dimension2df(400.0f, 400.0f));
			scene::IMeshSceneNode* node = device->getSceneManager()->addMeshSceneNode(mesh, 0, -1, position, rotation);
			mesh->drop();
			node->updateAbsolutePosition();
			return node;
		}

		scene::IMeshSceneNode* box(const vector3df& centre, const vector3df& size)
		{
			scene::IMesh* mesh = device->getSceneManager()->getGeometryCreator()->createCubeMesh(size);
			scene::IMeshSceneNode* node = device->getSceneManager()->addMeshSceneNode(mesh, 0, -1, centre);
			mesh->drop();
			node->updateAbsolutePosition();
			return node;
		}

		BodyId floor(float y = 0.0f)
		{
			return physics->addStaticMesh(plane(vector3df(0.0f, y, 0.0f)));
		}

		CharacterController& character(const CharacterSettings& settings, const vector3df& at)
		{
			characters.push_back(std::unique_ptr<CharacterController>(physics->addCharacter(settings)));
			characters.back()->warp(at);
			return *characters.back();
		}

		void run(float seconds)
		{
			int frames = (int)std::lround(seconds / FRAME);
			for (int i = 0; i < frames; ++i)
				physics->step(FRAME);
		}

		IrrlichtDevice* device;
		PhysicsWorld* physics;
		std::vector<std::unique_ptr<CharacterController> > characters;
	};

	void baseline(const char* name, float value)
	{
		std::printf("BASELINE %s %.4f\n", name, value);
	}
}

TEST_CASE("physics: a character falls under gravity and lands on a static mesh")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 40.0f, 0.0f));
	CHECK_FALSE(character.onGround());
	world.run(0.25f);
	CHECK_FALSE(character.onGround());
	world.run(3.0f);
	baseline("rest_height", character.position().Y);
	CHECK(character.onGround());
	CHECK(character.position().Y == doctest::Approx(4.75f).epsilon(0.01));
}

TEST_CASE("physics: falling speed")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 900.0f, 0.0f));
	world.run(1.0f);
	float a = character.position().Y;
	world.run(0.5f);
	float b = character.position().Y;
	baseline("fall_speed_after_1s", (a - b) / 0.5f);
	CHECK((a - b) / 0.5f == doctest::Approx(188.75f).epsilon(0.05));
}

TEST_CASE("physics: walking speed matches setWalk")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	world.run(2.0f);
	float start = character.position().X;
	character.setWalk(vector3df(54.0f, 0.0f, 0.0f));
	world.run(1.0f);
	float speed = character.position().X - start;
	baseline("walk_speed_for_54", speed);
	CHECK(speed == doctest::Approx(54.0f).epsilon(0.05));
}

TEST_CASE("physics: a jump rises to an apex and comes back down")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	world.run(2.0f);
	float rest = character.position().Y;
	character.jump();
	float apex = rest;
	int apexFrame = 0;
	int frames = 0;
	bool landed = false;
	while (frames < 300)
	{
		world.physics->step(FRAME);
		++frames;
		float y = character.position().Y;
		if (y > apex)
		{
			apex = y;
			apexFrame = frames;
		}
		if (frames > 5 && character.onGround())
		{
			landed = true;
			break;
		}
	}
	baseline("jump_apex_height", apex - rest);
	baseline("jump_apex_time", apexFrame * FRAME);
	baseline("jump_air_time", frames * FRAME);
	CHECK(landed);
	CHECK(apex - rest == doctest::Approx(11.5f).epsilon(0.10));
	CHECK(apexFrame * FRAME == doctest::Approx(0.383f).epsilon(0.10));
	CHECK(frames * FRAME == doctest::Approx(0.8f).epsilon(0.10));
}

TEST_CASE("physics: slopes either side of the maximum")
{
	for (int degrees : { 44, 46 })
	{
		TestWorld world;
		float angle = degrees * PI_F / 180.0f;
		world.physics->addStaticMesh(world.plane(vector3df(), vector3df(0.0f, 0.0f, (float)degrees)));
		CharacterSettings settings = playerSettings();
		settings.maxSlope = 45.0f * PI_F / 180.0f;
		CharacterController& character = world.character(settings, vector3df(-20.0f, -20.0f * std::tan(angle) + 10.0f, 0.0f));
		world.run(2.0f);
		vector3df start = character.position();
		character.setWalk(vector3df(30.0f, 0.0f, 0.0f));
		world.run(2.0f);
		vector3df end = character.position();
		char name[64];
		std::snprintf(name, sizeof(name), "slope_%d_rise", degrees);
		baseline(name, end.Y - start.Y);
		std::snprintf(name, sizeof(name), "slope_%d_run", degrees);
		baseline(name, end.X - start.X);
		if (degrees < 45)
			CHECK(end.Y - start.Y > 20.0f);
		else
			CHECK(end.Y - start.Y < 0.0f);
	}
}

TEST_CASE("physics: a character standing on a walkable slope stays put")
{
	TestWorld world;
	world.physics->addStaticMesh(world.plane(vector3df(), vector3df(0.0f, 0.0f, 30.0f)));
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	world.run(2.0f);
	vector3df start = character.position();
	world.run(2.0f);
	baseline("slope_30_creep", (character.position() - start).getLength());
	CHECK(character.onGround());
	CHECK((character.position() - start).getLength() < 0.05f);
}

TEST_CASE("physics: ledges at and above the step height")
{
	for (float height : { 0.35f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f })
	{
		TestWorld world;
		world.floor();
		world.physics->addStaticMesh(world.box(vector3df(30.0f, height - 0.5f, 0.0f), vector3df(40.0f, 1.0f, 40.0f)));
		CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
		world.run(2.0f);
		float rest = character.position().Y;
		character.setWalk(vector3df(30.0f, 0.0f, 0.0f));
		world.run(1.0f);
		char name[64];
		std::snprintf(name, sizeof(name), "ledge_%.2f_rise", height);
		baseline(name, character.position().Y - rest);
		std::snprintf(name, sizeof(name), "ledge_%.2f_x", height);
		baseline(name, character.position().X);
		if (height <= 0.35f)
			CHECK(character.position().Y - rest == doctest::Approx(height).epsilon(0.05));
		else
		{
			CHECK(std::fabs(character.position().Y - rest) < 0.01f);
			CHECK(character.position().X == doctest::Approx(7.5f).epsilon(0.05));
		}
	}
}

TEST_CASE("physics: warp places the character exactly")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(10.0f, 20.0f, 30.0f));
	CHECK(character.position().X == doctest::Approx(10.0f));
	CHECK(character.position().Y == doctest::Approx(20.0f));
	CHECK(character.position().Z == doctest::Approx(30.0f));
	character.warp(vector3df(-5.0f, 7.0f, 2.0f));
	CHECK(character.position().X == doctest::Approx(-5.0f));
	CHECK(character.position().Y == doctest::Approx(7.0f));
}

TEST_CASE("physics: a dynamic box falls and settles on a static mesh")
{
	TestWorld world;
	world.floor();
	scene::IMeshSceneNode* crate = world.box(vector3df(0.0f, 30.0f, 0.0f), vector3df(4.0f, 4.0f, 4.0f));
	world.physics->addDynamicBox(crate, 64.0f * 0.001f);
	world.run(4.0f);
	float settled = crate->getPosition().Y;
	world.run(0.5f);
	baseline("box_rest_height", settled);
	baseline("box_drift_last_half_second", std::fabs(crate->getPosition().Y - settled));
	CHECK(settled == doctest::Approx(2.0f).epsilon(0.05));
	CHECK(std::fabs(crate->getPosition().Y - settled) < 0.01f);
}

TEST_CASE("physics: a long frame catches up at most eight fixed steps")
{
	TestWorld world;
	world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	world.run(2.0f);
	character.setWalk(vector3df(60.0f, 0.0f, 0.0f));
	float start = character.position().X;
	world.physics->step(1.0f);
	baseline("long_frame_walk", character.position().X - start);
	CHECK(character.position().X - start == doctest::Approx(8.0f).epsilon(0.05));
}

TEST_CASE("physics: a box starts where its node was just moved and scaled")
{
	TestWorld world;
	world.floor();
	scene::IMeshSceneNode* crate = world.box(vector3df(), vector3df(2.0f, 2.0f, 2.0f));
	crate->setPosition(vector3df(15.0f, 30.0f, -5.0f));
	crate->setScale(vector3df(2.0f, 2.0f, 2.0f));
	world.physics->addDynamicBox(crate, 0.064f);
	world.physics->step(FRAME);
	CHECK(crate->getPosition().X == doctest::Approx(15.0f));
	CHECK(crate->getPosition().Y == doctest::Approx(30.0f).epsilon(0.01));
	CHECK(crate->getPosition().Z == doctest::Approx(-5.0f));
	world.run(4.0f);
	CHECK(crate->getPosition().Y == doctest::Approx(2.0f).epsilon(0.05));
}

TEST_CASE("physics: a dynamic box lands on another dynamic box")
{
	TestWorld world;
	world.floor();
	scene::IMeshSceneNode* lower = world.box(vector3df(0.0f, 10.0f, 0.0f), vector3df(4.0f, 4.0f, 4.0f));
	scene::IMeshSceneNode* upper = world.box(vector3df(0.0f, 30.0f, 0.0f), vector3df(4.0f, 4.0f, 4.0f));
	world.physics->addDynamicBox(lower, 0.064f);
	world.physics->addDynamicBox(upper, 0.064f);
	world.run(4.0f);
	baseline("stacked_lower_y", lower->getPosition().Y);
	baseline("stacked_upper_y", upper->getPosition().Y);
	CHECK(lower->getPosition().Y == doctest::Approx(2.0f).epsilon(0.05));
	CHECK(upper->getPosition().Y == doctest::Approx(6.0f).epsilon(0.05));
}

TEST_CASE("physics: characters block each other unless one ignores characters")
{
	for (bool collides : { true, false })
	{
		TestWorld world;
		world.floor();
		CharacterController& walker = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
		CharacterSettings standing = playerSettings();
		standing.collidesWithCharacters = collides;
		CharacterController& other = world.character(standing, vector3df(12.0f, 10.0f, 0.0f));
		world.run(2.0f);
		walker.setWalk(vector3df(20.0f, 0.0f, 0.0f));
		world.run(2.0f);
		baseline(collides ? "blocked_walker_x" : "ghost_walker_x", walker.position().X);
		baseline(collides ? "blocked_other_x" : "ghost_other_x", other.position().X);
		CHECK(other.position().X == doctest::Approx(12.0f));
		if (collides)
			CHECK(walker.position().X == doctest::Approx(7.0f).epsilon(0.05));
		else
			CHECK(walker.position().X == doctest::Approx(40.0f).epsilon(0.05));
	}
}

TEST_CASE("physics: a removed body no longer collides")
{
	TestWorld world;
	BodyId ground = world.floor();
	CharacterController& character = world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	world.run(2.0f);
	CHECK(character.onGround());
	world.physics->removeBody(ground);
	world.run(2.0f);
	baseline("after_removal_y", character.position().Y);
	CHECK(character.position().Y < -20.0f);
}

TEST_CASE("physics: a ray hits the floor below it and misses empty space")
{
	TestWorld world;
	BodyId ground = world.floor(3.0f);
	RayHit hit;
	CHECK(world.physics->raycast(vector3df(5.0f, 50.0f, -5.0f), vector3df(5.0f, -50.0f, -5.0f), hit));
	CHECK(hit.body == ground);
	CHECK(hit.point.X == doctest::Approx(5.0f));
	CHECK(hit.point.Y == doctest::Approx(3.0f));
	CHECK(hit.point.Z == doctest::Approx(-5.0f));
	CHECK(hit.normal.Y == doctest::Approx(1.0f));
	CHECK_FALSE(world.physics->raycast(vector3df(5.0f, 50.0f, -5.0f), vector3df(5.0f, 10.0f, -5.0f), hit));
	scene::IMeshSceneNode* moved = world.plane(vector3df());
	moved->setPosition(vector3df(1000.0f, 7.0f, 1000.0f));
	world.physics->addStaticMesh(moved);
	CHECK(world.physics->raycast(vector3df(1000.0f, 50.0f, 1000.0f), vector3df(1000.0f, -50.0f, 1000.0f), hit));
	CHECK(hit.point.Y == doctest::Approx(7.0f));
}

TEST_CASE("physics: stats count bodies and characters")
{
	TestWorld world;
	BodyId ground = world.floor();
	world.physics->addDynamicBox(world.box(vector3df(0.0f, 30.0f, 0.0f), vector3df(4.0f, 4.0f, 4.0f)), 0.064f);
	world.character(playerSettings(), vector3df(0.0f, 10.0f, 0.0f));
	PhysicsStats stats = world.physics->stats();
	CHECK(stats.staticBodies == 1);
	CHECK(stats.dynamicBodies == 1);
	CHECK(stats.characters == 1);
	world.physics->removeBody(ground);
	world.characters.clear();
	stats = world.physics->stats();
	CHECK(stats.staticBodies == 0);
	CHECK(stats.characters == 0);
}
