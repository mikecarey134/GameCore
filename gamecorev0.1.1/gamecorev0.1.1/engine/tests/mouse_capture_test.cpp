#include "doctest.h"
#include "MouseCapture.h"
#include <ICursorControl.h>

using irr::core::position2d;
using irr::core::position2df;

namespace
{
	class FakeCursor : public irr::gui::ICursorControl
	{
	public:
		FakeCursor(int width, int height) : width_(width), height_(height), visible_(true), position_(0, 0) {}

		void setVisible(bool visible) override { visible_ = visible; }
		bool isVisible() const override { return visible_; }
		void setPosition(const position2d<irr::f32>& pos) override { setPosition(pos.X, pos.Y); }
		void setPosition(irr::f32 x, irr::f32 y) override { setPosition((irr::s32)(x * width_), (irr::s32)(y * height_)); }
		void setPosition(const position2d<irr::s32>& pos) override { position_ = pos; }
		void setPosition(irr::s32 x, irr::s32 y) override { position_ = position2d<irr::s32>(x, y); }
		const position2d<irr::s32>& getPosition() override { return position_; }
		position2d<irr::f32> getRelativePosition() override
		{
			return position2d<irr::f32>(position_.X / (irr::f32)width_, position_.Y / (irr::f32)height_);
		}
		void setReferenceRect(irr::core::rect<irr::s32>*) override {}

		void move(int dx, int dy) { position_ += position2d<irr::s32>(dx, dy); }

	private:
		int width_;
		int height_;
		bool visible_;
		position2d<irr::s32> position_;
	};

	position2df deltaAfterMove(int width, int height, int dx, int dy)
	{
		FakeCursor cursor(width, height);
		MouseCapture capture(&cursor);
		capture.setCaptured(true);
		cursor.move(dx, dy);
		capture.beginFrame();
		return capture.delta();
	}
}

TEST_CASE("mouse capture: an uncaptured mouse gives no delta and keeps the cursor visible")
{
	FakeCursor cursor(800, 600);
	MouseCapture capture(&cursor);
	cursor.move(30, 30);
	capture.beginFrame();
	CHECK(capture.delta() == position2df(0.0f, 0.0f));
	CHECK(cursor.isVisible());
}

TEST_CASE("mouse capture: the delta is the pixel distance moved from the centre")
{
	CHECK(deltaAfterMove(800, 600, 20, -10) == position2df(20.0f, -10.0f));
}

TEST_CASE("mouse capture: the same pixel movement gives the same delta at any window size")
{
	CHECK(deltaAfterMove(1280, 720, 20, -10) == deltaAfterMove(800, 600, 20, -10));
	CHECK(deltaAfterMove(1281, 721, 7, 3) == position2df(7.0f, 3.0f));
}

TEST_CASE("mouse capture: capturing hides the cursor and each frame recentres it")
{
	FakeCursor cursor(800, 600);
	MouseCapture capture(&cursor);
	capture.setCaptured(true);
	CHECK_FALSE(cursor.isVisible());
	cursor.move(12, 0);
	capture.beginFrame();
	CHECK(capture.delta() == position2df(12.0f, 0.0f));
	capture.beginFrame();
	CHECK(capture.delta() == position2df(0.0f, 0.0f));
	CHECK(cursor.getPosition() == position2d<irr::s32>(400, 300));
}
