#include "MouseCapture.h"
#include <ICursorControl.h>

MouseCapture::MouseCapture(irr::gui::ICursorControl* cursor)
	: cursor_(cursor), captured_(false), delta_(0.0f, 0.0f), centre_(0, 0)
{
}

void MouseCapture::setCaptured(bool captured)
{
	if (captured == captured_)
		return;
	captured_ = captured;
	delta_ = irr::core::position2df(0.0f, 0.0f);
	cursor_->setVisible(!captured);
	if (captured)
		recentre();
}

void MouseCapture::recentre()
{
	cursor_->setPosition(0.5f, 0.5f);
	centre_ = cursor_->getPosition();
}

void MouseCapture::beginFrame()
{
	if (!captured_)
	{
		delta_ = irr::core::position2df(0.0f, 0.0f);
		return;
	}
	irr::core::position2di cursor = cursor_->getPosition();
	delta_ = irr::core::position2df((irr::f32)(cursor.X - centre_.X), (irr::f32)(cursor.Y - centre_.Y));
	recentre();
}
