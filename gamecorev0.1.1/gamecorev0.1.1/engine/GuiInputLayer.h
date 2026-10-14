#ifndef GUIINPUTLAYER_H
#define GUIINPUTLAYER_H

#include <functional>
#include "InputRouter.h"

namespace irr { namespace gui { class IGUIEnvironment; } }

class GuiInputLayer : public InputLayer
{
public:
	GuiInputLayer(irr::gui::IGUIEnvironment* gui, std::function<bool(const irr::SEvent&)> onGuiEvent,
		std::function<void()> onBack, std::function<bool()> active);

	bool onEvent(const irr::SEvent& event) override;
	bool isActive() const override { return active_(); }

private:
	irr::gui::IGUIEnvironment* gui_;
	std::function<bool(const irr::SEvent&)> onGuiEvent_;
	std::function<void()> onBack_;
	std::function<bool()> active_;
};

#endif
