#include "ConsoleLayer.h"
#include "console.h"

using namespace irr;

bool ConsoleLayer::onEvent(const SEvent& event)
{
	if (event.EventType == EET_LOG_TEXT_EVENT)
	{
		console_.logMessage_ANSI(event.LogEvent.Level, event.LogEvent.Text);
		return true;
	}

	if (actions_.pressed("console", event) && (!canToggle_ || canToggle_()))
	{
		if (!console_.isVisible())
		{
			console_.setVisible(true);
			return true;
		}
		else if (event.EventType != EET_KEY_INPUT_EVENT || !event.KeyInput.Control)
		{
			console_.setVisible(false);
			return true;
		}
	}

	if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		if (console_.isVisible())
		{
			if (event.KeyInput.Key == KEY_ESCAPE)
				return false;
			console_.handleKeyPress(event.KeyInput.Char, event.KeyInput.Key, event.KeyInput.Shift, event.KeyInput.Control);
			return true;
		}
	}

	if (event.EventType == EET_MOUSE_INPUT_EVENT)
		return console_.isVisible();

	return false;
}
