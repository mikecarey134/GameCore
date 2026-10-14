#include "GuiInputLayer.h"
#include <IGUIEnvironment.h>

using namespace irr;

GuiInputLayer::GuiInputLayer(gui::IGUIEnvironment* gui, std::function<bool(const SEvent&)> onGuiEvent,
	std::function<void()> onBack, std::function<bool()> active)
	: gui_(gui), onGuiEvent_(onGuiEvent), onBack_(onBack), active_(active)
{
}

bool GuiInputLayer::onEvent(const SEvent& event)
{
	if (event.EventType == EET_GUI_EVENT)
		return onGuiEvent_(event);

	if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_ESCAPE)
	{
		if (event.KeyInput.PressedDown)
			onBack_();
		return true;
	}

	if (event.EventType == EET_KEY_INPUT_EVENT || event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		gui_->postEventFromUser(event);
		return true;
	}
	return false;
}
