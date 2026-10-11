#include "MouseCapture.h"
#include <ICursorControl.h>

MouseCapture::MouseCapture(irr::gui::ICursorControl* cursor)
	: cursor_(cursor), captured_(false), delta_(0.0f, 0.0f)
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
		cursor_->setPosition(0.5f, 0.5f);
}

void MouseCapture::beginFrame()
{
	if (!captured_)
	{
		delta_ = irr::core::position2df(0.0f, 0.0f);
		return;
	}
	irr::core::position2df cursor = cursor_->getRelativePosition();
	delta_ = irr::core::position2df(cursor.X - 0.5f, cursor.Y - 0.5f);
	cursor_->setPosition(0.5f, 0.5f);
}
