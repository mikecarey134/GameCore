#include "doctest.h"
#include "OrbitCamera.h"

TEST_CASE("a new camera looks straight ahead at the default distance")
{
	OrbitCamera camera;
	CHECK(camera.getYaw() == doctest::Approx(0.0f));
	CHECK(camera.getPitch() == doctest::Approx(0.0f));
	CHECK(camera.getDistance() == doctest::Approx(10.0f));
}

TEST_CASE("moving the mouse right turns the camera by the sensitivity")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.1f, 0.0f));
	CHECK(camera.getYaw() == doctest::Approx(5.0f));
}

TEST_CASE("yaw keeps accumulating across updates")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.1f, 0.0f));
	camera.update(irr::core::position2df(0.2f, 0.0f));
	CHECK(camera.getYaw() == doctest::Approx(15.0f));
}

TEST_CASE("moving the mouse up raises the pitch")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.0f, -0.1f));
	CHECK(camera.getPitch() == doctest::Approx(5.0f));
}

TEST_CASE("pitch stops just short of straight down")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.0f, 3.0f));
	CHECK(camera.getPitch() == doctest::Approx(-89.9f));
}

TEST_CASE("pitch stops just short of 40 degrees up")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.0f, -3.0f));
	CHECK(camera.getPitch() == doctest::Approx(39.9f));
}

TEST_CASE("the sensitivity sets how far a mouse movement turns the camera")
{
	OrbitCamera camera;
	camera.setSensitivity(100.0f);
	camera.update(irr::core::position2df(0.1f, -0.1f));
	CHECK(camera.getYaw() == doctest::Approx(10.0f));
	CHECK(camera.getPitch() == doctest::Approx(10.0f));
}

TEST_CASE("zoom changes the distance by the wheel amount")
{
	OrbitCamera camera;
	camera.zoom(3.0f);
	CHECK(camera.getDistance() == doctest::Approx(13.0f));
	camera.zoom(-1.0f);
	CHECK(camera.getDistance() == doctest::Approx(12.0f));
}

TEST_CASE("zoom stops at 5 and 50")
{
	OrbitCamera camera;
	camera.zoom(-100.0f);
	CHECK(camera.getDistance() == doctest::Approx(5.0f));
	camera.zoom(100.0f);
	CHECK(camera.getDistance() == doctest::Approx(50.0f));
}

TEST_CASE("reset faces the camera straight ahead but keeps the distance")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.3f, -0.2f));
	camera.zoom(5.0f);
	camera.reset();
	CHECK(camera.getYaw() == doctest::Approx(0.0f));
	CHECK(camera.getPitch() == doctest::Approx(0.0f));
	CHECK(camera.getDistance() == doctest::Approx(15.0f));
}

TEST_CASE("the camera sits behind the target at the default angles")
{
	OrbitCamera camera;
	camera.setTarget(irr::core::vector3df(0.0f, 0.0f, 0.0f));
	irr::core::vector3df position = camera.position();
	CHECK(position.X == doctest::Approx(-10.0f));
	CHECK(position.Y == doctest::Approx(0.0f));
	CHECK(position.Z == doctest::Approx(0.0f));
}

TEST_CASE("the position follows the yaw, pitch and distance of the current formula")
{
	OrbitCamera camera;
	camera.setTarget(irr::core::vector3df(1.0f, 2.0f, 3.0f));
	camera.update(irr::core::position2df(0.6f, -0.4f));
	camera.zoom(5.0f);
	irr::core::vector3df position = camera.position();
	CHECK(position.X == doctest::Approx(1.0f - 12.990381f));
	CHECK(position.Y == doctest::Approx(2.0f - 5.130302f));
	CHECK(position.Z == doctest::Approx(3.0f + 7.5f));
}

TEST_CASE("the look direction is the yaw and pitch vector the pick ray uses")
{
	OrbitCamera camera;
	camera.update(irr::core::position2df(0.6f, -0.4f));
	irr::core::vector3df look = camera.lookDirection();
	CHECK(look.X == doctest::Approx(0.866025f));
	CHECK(look.Y == doctest::Approx(0.342020f));
	CHECK(look.Z == doctest::Approx(-0.5f));
}

TEST_CASE("the move yaw is the horizontal heading of the look direction")
{
	OrbitCamera camera;
	CHECK(camera.getMoveYaw() == doctest::Approx(90.0f));
	camera.update(irr::core::position2df(1.8f, 0.0f));
	CHECK(camera.getMoveYaw() == doctest::Approx(180.0f));
}
