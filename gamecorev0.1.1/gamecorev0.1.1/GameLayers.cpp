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

bool MenuLayer::onEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_ESCAPE && 
			event.KeyInput.PressedDown == true)
		{//handle the ESC key to display the pause menu
			if(game_.pauseMenu || game_.console.isVisible())
			{
				game_.chatshown = false;
				game_.pauseMenu = false;
				game_.console.setVisible(false);
				
			}
		
			else if (game_.started && game_.player_->isDead())
				game_.goToMainMenu();
			else
				game_.pauseMenu = true;
				

			game_.engine_->play2D("sounds/button-20.wav",false);
		
		}
	}

	if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = game_.Context_.device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{
			case EGET_EDITBOX_ENTER:
				{
					game_.player_->setChatMessage("");
					game_.chatshown = false;
					
				}
				break;

			case EGET_SCROLL_BAR_CHANGED:
				if (id == consoleevent::GUI_ID_TRANSPARENCY_SCROLL_BAR)
				{
					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					game_.setSkinTransparency(pos, env->getSkin());
				}
				break;

			case EGET_BUTTON_CLICKED: //if one of our gui buttons is clicked handle it
				switch(id)
				{
				case consoleevent::GUI_ID_QUIT_BUTTON:
					game_.Context_.device->closeDevice();
					game_.engine_->play2D("sounds/button-25.wav",false);
					return true;

				case consoleevent::GUI_ID_START:

					game_.startPressed();
					return true;

				case consoleevent::GUI_ID_CREDITS:
					{
						if (!game_.creditsDisplayed)
						{
							game_.creditsDisplayed = true;
						}
					}
					return true;

				case consoleevent::GUI_ID_GOTO_MAIN_MENU:
					game_.goToMainMenu();
					return true;

				case consoleevent::GUI_ID_RESUME:
					if(game_.pauseMenu)//this fixes the menu sound bug Mike
						game_.engine_->play2D("sounds/button-25.wav",false);
					game_.pauseMenu = false;
					
					return true;
					
				case consoleevent::GUI_ID_FILE_OPEN_BUTTON:
					game_.Context_.listbox->addItem(L"File open");
					// There are some options for the file open dialog
					// We set the title, make it a modal window, and make sure
					// that the working directory is restored after the dialog
					// is finished.
					env->addFileOpenDialog(L"Please choose a file.", true, 0, -1, true);
					return true;

				default:
					return false;
				}
				break;

			case EGET_FILE_SELECTED:
				{
					// show the model filename, selected in the file dialog
					IGUIFileOpenDialog* dialog =
						(IGUIFileOpenDialog*)event.GUIEvent.Caller;
					game_.Context_.listbox->addItem(dialog->getFileName());
				}
				break;
			

			default:
				break;
			}
		}

	return false;
}

bool DebugLayer::onEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_F && 
			event.KeyInput.PressedDown == true && !game_.console.isVisible() && !game_.player_->isDead())
		{
			if(game_.player_->getIsLamp())
				game_.player_->setlamp(false);
			else
				game_.player_->setlamp(true);
		}

		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_T && 
			event.KeyInput.PressedDown == true && !game_.console.isVisible()&& !game_.pauseMenu)
		{//handle f3 key to display the debugging information
			if (game_.chatshown)
			{
				//chatshown = false;

			}
			else
				game_.chatshown = true;
			std::cout<<"T_Pressed!\n"<<"Value:"<<game_.chatshown<<"\n";
			
		}

		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_F3 && 
			event.KeyInput.PressedDown == true)
		{//handle f3 key to display the debugging information
			if(game_.debug)
				game_.debug = false;
			else
				game_.debug = true;
		}
	}
	return false;
}
