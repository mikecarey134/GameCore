#include "doctest.h"
#include "UiAnchor.h"

using irr::core::rect;
using irr::core::dimension2d;
using irr::s32;
using irr::u32;

namespace
{
	const rect<s32> hud(290, 530, 560, 570);

	rect<s32> shifted(const rect<s32>& r, s32 x, s32 y)
	{
		return rect<s32>(r.UpperLeftCorner.X + x, r.UpperLeftCorner.Y + y, r.LowerRightCorner.X + x, r.LowerRightCorner.Y + y);
	}
}

TEST_CASE("anchoring at the design size changes nothing for any anchor")
{
	const float anchors[] = { 0.0f, 0.5f, 1.0f };
	for (float x : anchors)
		for (float y : anchors)
			CHECK(anchored(hud, x, y, dimension2d<u32>(800, 600)) == hud);
}

TEST_CASE("anchoring at 1280x720 moves the rect by the anchored share of the extra space")
{
	const dimension2d<u32> screen(1280, 720);
	CHECK(anchored(hud, 0.0f, 0.0f, screen) == hud);
	CHECK(anchored(hud, 0.5f, 0.0f, screen) == shifted(hud, 240, 0));
	CHECK(anchored(hud, 1.0f, 0.0f, screen) == shifted(hud, 480, 0));
	CHECK(anchored(hud, 0.0f, 0.5f, screen) == shifted(hud, 0, 60));
	CHECK(anchored(hud, 0.5f, 0.5f, screen) == shifted(hud, 240, 60));
	CHECK(anchored(hud, 1.0f, 0.5f, screen) == shifted(hud, 480, 60));
	CHECK(anchored(hud, 0.0f, 1.0f, screen) == shifted(hud, 0, 120));
	CHECK(anchored(hud, 0.5f, 1.0f, screen) == shifted(hud, 240, 120));
	CHECK(anchored(hud, 1.0f, 1.0f, screen) == shifted(hud, 480, 120));
}

TEST_CASE("anchoring at 1920x1080 keeps the rect's size and moves it")
{
	const dimension2d<u32> screen(1920, 1080);
	CHECK(anchored(hud, 0.5f, 1.0f, screen) == shifted(hud, 560, 480));
	CHECK(anchored(hud, 1.0f, 0.0f, screen) == shifted(hud, 1120, 0));
	CHECK(anchored(hud, 0.5f, 0.5f, screen) == shifted(hud, 560, 240));
	CHECK(anchored(hud, 1.0f, 1.0f, screen).getSize() == hud.getSize());
}

TEST_CASE("an odd extra width rounds the centred offset down")
{
	CHECK(anchored(hud, 0.5f, 0.5f, dimension2d<u32>(1281, 721)) == shifted(hud, 240, 60));
}

TEST_CASE("a screen smaller than the design moves the rect back")
{
	CHECK(anchored(hud, 1.0f, 1.0f, dimension2d<u32>(640, 480)) == shifted(hud, -160, -120));
	CHECK(anchored(hud, 0.0f, 0.0f, dimension2d<u32>(640, 480)) == hud);
}

TEST_CASE("the letterbox is the whole screen at 4:3")
{
	CHECK(letterboxed(dimension2d<u32>(800, 600)) == rect<s32>(0, 0, 800, 600));
	CHECK(letterboxed(dimension2d<u32>(1600, 1200)) == rect<s32>(0, 0, 1600, 1200));
}

TEST_CASE("the letterbox is a centred 4:3 rect with bars at the sides on wide screens")
{
	CHECK(letterboxed(dimension2d<u32>(1280, 720)) == rect<s32>(160, 0, 1120, 720));
	CHECK(letterboxed(dimension2d<u32>(1920, 1080)) == rect<s32>(240, 0, 1680, 1080));
}

TEST_CASE("the letterbox has bars at the top and bottom on tall screens")
{
	CHECK(letterboxed(dimension2d<u32>(800, 1000)) == rect<s32>(0, 200, 800, 800));
}
