#include "doctest.h"
#include "net/Interpolator.h"

namespace
{
	void add(Interpolator& interpolator, uint16_t seq, float x, float z, float yaw, float time)
	{
		const float position[3] = { x, 1.0f, z };
		interpolator.addSnapshot(seq, position, yaw, time);
	}

	Pose at(const Interpolator& interpolator, float renderTime)
	{
		Pose pose = Pose();
		REQUIRE(interpolator.sample(renderTime, pose));
		return pose;
	}
}

TEST_CASE("between two snapshots position and yaw move in a straight line")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 4.0f, -2.0f, 40.0f, 1.05f);
	Pose pose = at(interpolator, 1.0125f);
	CHECK(pose.position[0] == doctest::Approx(1.0f));
	CHECK(pose.position[1] == doctest::Approx(1.0f));
	CHECK(pose.position[2] == doctest::Approx(-0.5f));
	CHECK(pose.yaw == doctest::Approx(10.0f));
	pose = at(interpolator, 1.025f);
	CHECK(pose.position[0] == doctest::Approx(2.0f));
	CHECK(pose.yaw == doctest::Approx(20.0f));
}

TEST_CASE("a render time before every snapshot uses the oldest")
{
	Interpolator interpolator;
	add(interpolator, 1, 3.0f, 0.0f, 5.0f, 1.0f);
	add(interpolator, 2, 6.0f, 0.0f, 9.0f, 1.05f);
	Pose pose = at(interpolator, 0.5f);
	CHECK(pose.position[0] == doctest::Approx(3.0f));
	CHECK(pose.yaw == doctest::Approx(5.0f));
}

TEST_CASE("a render time after every snapshot holds the newest")
{
	Interpolator interpolator;
	add(interpolator, 1, 3.0f, 0.0f, 5.0f, 1.0f);
	add(interpolator, 2, 6.0f, 0.0f, 9.0f, 1.05f);
	Pose pose = at(interpolator, 9.0f);
	CHECK(pose.position[0] == doctest::Approx(6.0f));
	CHECK(pose.yaw == doctest::Approx(9.0f));
}

TEST_CASE("a render time exactly on a snapshot returns that snapshot")
{
	Interpolator interpolator;
	add(interpolator, 1, 3.0f, 0.0f, 5.0f, 1.0f);
	add(interpolator, 2, 6.0f, 0.0f, 9.0f, 1.05f);
	add(interpolator, 3, 8.0f, 0.0f, 12.0f, 1.10f);
	CHECK(at(interpolator, 1.0f).position[0] == doctest::Approx(3.0f));
	CHECK(at(interpolator, 1.05f).position[0] == doctest::Approx(6.0f));
	CHECK(at(interpolator, 1.10f).position[0] == doctest::Approx(8.0f));
}

TEST_CASE("with three snapshots the pair around the render time is used")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 2.0f, 0.0f, 0.0f, 1.05f);
	add(interpolator, 3, 2.0f, 8.0f, 0.0f, 1.10f);
	Pose pose = at(interpolator, 1.075f);
	CHECK(pose.position[0] == doctest::Approx(2.0f));
	CHECK(pose.position[2] == doctest::Approx(4.0f));
	pose = at(interpolator, 1.025f);
	CHECK(pose.position[0] == doctest::Approx(1.0f));
	CHECK(pose.position[2] == doctest::Approx(0.0f));
}

TEST_CASE("only the last three snapshots are kept")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 1.0f, 0.0f, 0.0f, 1.05f);
	add(interpolator, 3, 2.0f, 0.0f, 0.0f, 1.10f);
	add(interpolator, 4, 3.0f, 0.0f, 0.0f, 1.15f);
	CHECK(interpolator.size() == Interpolator::CAPACITY);
	CHECK(at(interpolator, 0.0f).position[0] == doctest::Approx(1.0f));
}

TEST_CASE("a jump of more than ten units snaps to the newer snapshot instead of sliding")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 30.0f, 0.0f, 90.0f, 1.05f);
	Pose pose = at(interpolator, 1.01f);
	CHECK(pose.position[0] == doctest::Approx(30.0f));
	CHECK(pose.yaw == doctest::Approx(90.0f));
	CHECK(at(interpolator, 1.0f).position[0] == doctest::Approx(0.0f));
}

TEST_CASE("a step just over ten units snaps")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 11.0f, 0.0f, 0.0f, 1.05f);
	CHECK(at(interpolator, 1.01f).position[0] == doctest::Approx(11.0f));
}

TEST_CASE("a step of exactly ten units still slides")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 2, 6.0f, 8.0f, 0.0f, 1.05f);
	Pose pose = at(interpolator, 1.025f);
	CHECK(pose.position[0] == doctest::Approx(3.0f));
	CHECK(pose.position[2] == doctest::Approx(4.0f));
}

TEST_CASE("an older or repeated sequence number is dropped")
{
	Interpolator interpolator;
	add(interpolator, 5, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 4, 9.0f, 0.0f, 0.0f, 1.05f);
	add(interpolator, 5, 9.0f, 0.0f, 0.0f, 1.05f);
	CHECK(interpolator.size() == 1);
	CHECK(at(interpolator, 2.0f).position[0] == doctest::Approx(0.0f));
}

TEST_CASE("sequence numbers keep counting across the wrap at 65535")
{
	CHECK(isNewerSeq(0, 65535));
	CHECK(isNewerSeq(10, 65530));
	CHECK_FALSE(isNewerSeq(65535, 0));
	CHECK_FALSE(isNewerSeq(7, 7));
	CHECK(isNewerSeq(8, 7));
	CHECK_FALSE(isNewerSeq(7, 8));

	Interpolator interpolator;
	add(interpolator, 65535, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 0, 2.0f, 0.0f, 0.0f, 1.05f);
	CHECK(interpolator.size() == 2);
	CHECK(at(interpolator, 1.025f).position[0] == doctest::Approx(1.0f));
}

TEST_CASE("yaw turns the short way around")
{
	Interpolator interpolator;
	add(interpolator, 1, 0.0f, 0.0f, 350.0f, 1.0f);
	add(interpolator, 2, 0.0f, 0.0f, 10.0f, 1.05f);
	float yaw = at(interpolator, 1.025f).yaw;
	CHECK((yaw == doctest::Approx(0.0f) || yaw == doctest::Approx(360.0f)));
	float later = at(interpolator, 1.0375f).yaw;
	CHECK((later == doctest::Approx(5.0f) || later == doctest::Approx(365.0f)));
}

TEST_CASE("reset forgets every snapshot and the next one starts fresh")
{
	Interpolator interpolator;
	add(interpolator, 9, 0.0f, 0.0f, 0.0f, 1.0f);
	add(interpolator, 10, 1.0f, 0.0f, 0.0f, 1.05f);
	interpolator.reset();
	CHECK(interpolator.size() == 0);
	Pose pose;
	CHECK_FALSE(interpolator.sample(1.0f, pose));
	add(interpolator, 3, 50.0f, 0.0f, 0.0f, 2.0f);
	CHECK(interpolator.size() == 1);
	CHECK(at(interpolator, 1.0f).position[0] == doctest::Approx(50.0f));
}

TEST_CASE("one snapshot is returned whatever the render time")
{
	Interpolator interpolator;
	add(interpolator, 1, 7.0f, 3.0f, 45.0f, 1.0f);
	CHECK(at(interpolator, 0.0f).position[0] == doctest::Approx(7.0f));
	CHECK(at(interpolator, 1.0f).position[2] == doctest::Approx(3.0f));
	CHECK(at(interpolator, 5.0f).yaw == doctest::Approx(45.0f));
}
