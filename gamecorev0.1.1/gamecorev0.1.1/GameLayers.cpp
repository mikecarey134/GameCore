#include "GameLayers.h"
#include "GameFlow.h"
#include "InventoryScreen.h"

using namespace irr;

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
				if (id == GameFlow::GUI_ID_TRANSPARENCY_SCROLL_BAR)
				{
					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					game_.setSkinTransparency(pos, env->getSkin());
				}
				break;

			case EGET_BUTTON_CLICKED: //if one of our gui buttons is clicked handle it
				switch(id)
				{
				case GameFlow::GUI_ID_QUIT_BUTTON:
					game_.Context_.device->closeDevice();
					game_.engine_->play2D("sounds/button-25.wav",false);
					return true;

				case GameFlow::GUI_ID_START:

					game_.startPressed();
					return true;

				case GameFlow::GUI_ID_CREDITS:
					{
						if (!game_.creditsDisplayed)
						{
							game_.creditsDisplayed = true;
						}
					}
					return true;

				case GameFlow::GUI_ID_GOTO_MAIN_MENU:
					game_.goToMainMenu();
					return true;

				case GameFlow::GUI_ID_RESUME:
					if(game_.pauseMenu)//this fixes the menu sound bug Mike
						game_.engine_->play2D("sounds/button-25.wav",false);
					game_.pauseMenu = false;
					
					return true;
					
				case GameFlow::GUI_ID_FILE_OPEN_BUTTON:
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

bool GameplayLayer::onEvent(const SEvent& event)
{
	//test for our mouse events
	if (event.EventType == EET_MOUSE_INPUT_EVENT && !game_.pauseMenu && !game_.console.isVisible() )
	{
		switch(event.MouseInput.Event)
		{
		case EMIE_RMOUSE_PRESSED_DOWN:
			if (game_.player_->isClueInRange() && !game_.player_->isDead())
			{
				//Iterate through the list of clues until we find the one the player is looking at
				std::list<ClueObject*>::iterator obj = game_.clueObjects_.begin();
				while(obj != game_.clueObjects_.end() &&
					  (*obj)->getModel()->getName() != game_.player_->getSelectedNode()->getName())
					++obj;

				if (obj != game_.clueObjects_.end())
				{
					game_.player_->addClue(*obj);
					(*obj)->eraseModelFromWorld();
				}
			}
			break;

		case EMIE_LMOUSE_PRESSED_DOWN:
			if (!game_.player_->isDead())
			{
				game_.player_->attack();
				
				if(game_.player_->enemyNPCInRange()/* && !npc_->isDead()*/)
				{
					game_.npc_->damage();
					//nodeName = npc_->characterModel_->getName();
				}
				else if (game_.player_->enemyPlayerInRange())
				{
					game_.findRemotePlayer(game_.player_->getSelectedNodeName())->damage();
					//findRemotePlayer(player_->getSelectedNodeName())->damage();
				}
			}
			break;

		case EMIE_LMOUSE_LEFT_UP:
			if (!game_.player_->isDead())
				game_.player_->idle();
			break;

		default:
			break;
		}
	}

	//if we scroll the mouse wheel we get closer or farther away from our player
	if(event.EventType == EET_MOUSE_INPUT_EVENT && event.MouseInput.Event == EMIE_MOUSE_WHEEL)
		game_.player_->setCamDist(event.MouseInput.Wheel);

	if(event.EventType == EET_KEY_INPUT_EVENT && !game_.console.isVisible() && !game_.pauseMenu && !game_.player_->isDead() && !game_.chatshown)  //handle player movement keyboard controls
	{
		player* thePlayer = game_.player_;
		if (input_.isKeyDown(KEY_SPACE))
			thePlayer->jump();

		if (input_.isKeyDown(KEY_KEY_W))		{ thePlayer->setZDir(-thePlayer->getPlayerSpeed()); thePlayer->forward(); }
		else if(input_.isKeyDown(KEY_KEY_S))	{ thePlayer->setZDir(thePlayer->getPlayerSpeed()); }
		else								{ thePlayer->setZDir(0.0f); /*player_->idle();*/ }

		if (input_.isKeyDown(KEY_KEY_A))		{ thePlayer->setXDir(thePlayer->getPlayerSpeed()); }
		else if(input_.isKeyDown(KEY_KEY_D))	{ thePlayer->setXDir(-thePlayer->getPlayerSpeed()); }
		else								{ thePlayer->setXDir(0.0f); /*player_->idle();*/}

		if(input_.isKeyUp(KEY_SPACE)&&input_.isKeyUp(KEY_KEY_W)&&input_.isKeyUp(KEY_KEY_S)&&input_.isKeyUp(KEY_KEY_A)&&input_.isKeyUp(KEY_KEY_D)){thePlayer->idle();}
	}

	return false;
}
