#ifndef MOUSECAPTURE_H
#define MOUSECAPTURE_H

#include <position2d.h>

namespace irr { namespace gui { class ICursorControl; } }

class MouseCapture
{
public:
	explicit MouseCapture(irr::gui::ICursorControl* cursor);

	void setCaptured(bool captured);
	bool isCaptured() const { return captured_; }
	void beginFrame();
	irr::core::position2df delta() const { return delta_; }

private:
	irr::gui::ICursorControl* cursor_;
	bool captured_;
	irr::core::position2df delta_;
};

#endif
