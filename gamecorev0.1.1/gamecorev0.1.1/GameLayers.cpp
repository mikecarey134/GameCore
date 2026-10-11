#include "GameLayers.h"
#include "consoleevent.h"

using namespace irr;

bool ConsoleLayer::onEvent(const SEvent& event)
{
	IC_Console& console = game_.getConsole();

	if (event.EventType == EET_LOG_TEXT_EVENT)
	{
		console.logMessage_ANSI(event.LogEvent.Level, event.LogEvent.Text);
		return true;
	}

	if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		if (event.KeyInput.Key == IC_Console::IC_KEY_TILDE && !game_.getPaused())
		{
			if (!console.isVisible())
			{
				console.setVisible(true);
				return true;
			}
			else if (!event.KeyInput.Control)
			{
				console.setVisible(false);
				return true;
			}
		}
		if (console.isVisible())
		{
			if (event.KeyInput.Key == KEY_ESCAPE)
				return false;
			console.handleKeyPress(event.KeyInput.Char, event.KeyInput.Key, event.KeyInput.Shift, event.KeyInput.Control);
			return true;
		}
	}

	if (event.EventType == EET_MOUSE_INPUT_EVENT)
		return console.isVisible();

	return false;
}
