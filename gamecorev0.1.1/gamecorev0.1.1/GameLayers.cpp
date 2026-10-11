#include "GameLayers.h"
#include "consoleevent.h"
#include "InventoryScreen.h"

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

bool NoteLayer::onEvent(const SEvent& event)
{
	bool dismissPress = (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown) ||
		(event.EventType == EET_MOUSE_INPUT_EVENT && event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN);
	if (!dismissPress)
		return false;

	bool dismissed = false;
	std::list<ClueObject*>& clues = game_.getClueObjects();
	for (std::list<ClueObject*>::iterator clue = clues.begin(); clue != clues.end(); ++clue)
		if ((*clue)->isNoteShowing())
		{
			(*clue)->hideNote();
			dismissed = true;
		}
	return dismissed;
}

bool InventoryLayer::onEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_E &&
		event.KeyInput.PressedDown && !game_.getPaused() && !game_.isConsoleVisible())
	{
		sound_->play2D("sounds/button-20.wav", false);
		if (game_.getIsInventory())
		{
			game_.setIsInventory(false);
			if (game_.getInventoryScreen())
				game_.getInventoryScreen()->close();
		}
		else
			game_.setIsInventory(true);
	}

	return game_.getIsInventory() && game_.getInventoryScreen() && game_.getInventoryScreen()->onEvent(event);
}
