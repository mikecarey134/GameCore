/************************************************************************/
/*
    consoleevent.cpp
	Purpose: handles all events in the game
	By:Michael Carey && Dan Brown

*/
/************************************************************************/


#include"consoleevent.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace irrklang;




consoleevent::consoleevent(irr::IrrlichtDevice *device, irr::gui::IGUIEnvironment* guienv,
						   irr::video::IVideoDriver* driver, SAppContext & Context,GUI* gui,
						   irrklang::ISoundEngine* engine, player* thePlayer, irrBulletWorld* world, 
						   NPC* npc, mapLoader* currentMap, InternalServer* client):
							device_(device),guienv_(guienv),driver_(driver),Context_(Context),gui_(gui),debug(false),							
							pauseMenu(false), started(false), chatshown(false),creditsDisplayed(false), inventory(false), engine_(engine), 
							player_(thePlayer), world_(world), npc_(npc), currentMap_(currentMap), client_(client)
{
	
	console.addExtensions(thePlayer,world,device,currentMap_);//register the player in the console
									//add world psychics to the console
	console.loadDefaultCommands(device);//load the default commands help echo etc
	console.initializeConsole(guienv, dimension2d<s32>(800,500));//prepare the console
	console.setVisible(false);

	for (u32 i=0; i < KEY_KEY_CODES_COUNT; ++i)
		KeyIsDown[i] = false;

}
consoleevent::~consoleevent(){}

bool consoleevent::OnEvent(const irr::SEvent& event)
{

	//test for our mouse events
	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT && !pauseMenu && !console.isVisible() )
	{
		switch(event.MouseInput.Event)
		{
		case EMIE_RMOUSE_PRESSED_DOWN:
			MouseState.RightButtonDown = true;

			if (player_->isClueInRange() && !player_->isDead())
			{
				//Iterate through the list of clues until we find the one the player is looking at
				std::list<ClueObject*>::iterator obj = clueObjects_.begin();
				ClueObject* clueHolder = *obj;
				while(clueHolder->getModel()->getName() != player_->getSelectedNode()->getName() &&
					  obj != clueObjects_.end())
				{
					++obj;
					clueHolder = *obj;
				}

				if (obj != clueObjects_.end())
				{
					player_->addClue(clueHolder);
					clueHolder->eraseModelFromWorld();
				}
			}
			

			//cout<<"Right Mouse CLick!\n";
			break;

		case EMIE_RMOUSE_LEFT_UP:
			MouseState.RightButtonDown = false;
			break;

		//handle mouse events  
		case EMIE_LMOUSE_PRESSED_DOWN:
			MouseState.LeftButtonDown = true;
			if (!player_->isDead())
			{
				player_->attack();
				
				if(player_->enemyNPCInRange()/* && !npc_->isDead()*/)
				{
					npc_->damage();
					//nodeName = npc_->characterModel_->getName();
				}
				else if (player_->enemyPlayerInRange())
				{
					findRemotePlayer(player_->getSelectedNodeName())->damage();
				}
			}
			

			//cout<<"Left Mouse CLick!\n";
			break;

		case EMIE_LMOUSE_LEFT_UP:
			MouseState.LeftButtonDown = false;
			if (!player_->isDead())
				player_->idle();
			break;
		case EMIE_MOUSE_MOVED:
			MouseState.Position.X = event.MouseInput.X;
			MouseState.Position.Y = event.MouseInput.Y;
			break;

		default:
			// We won't use the wheel
			break;
		}
	}

	//if we scroll the mouse wheel we get closer or farther away from our player
	if(event.EventType == irr::EET_MOUSE_INPUT_EVENT && event.MouseInput.Event == EMIE_MOUSE_WHEEL){
	
		player_->setCamDist(event.MouseInput.Wheel);
	}

	if(event.EventType == irr::EET_KEY_INPUT_EVENT  )
	{
		KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

		if(!console.isVisible() && !pauseMenu && !player_->isDead())//handle player movement keyboard controls
		{
			if (IsKeyDown(KEY_SPACE))
				player_->jump();

			if (IsKeyDown(KEY_KEY_W))		{ player_->setZDir(-player_->getPlayerSpeed()); player_->forward(); }
			else if(IsKeyDown(KEY_KEY_S))	{ player_->setZDir(player_->getPlayerSpeed()); }
			else							{ player_->setZDir(0.0f); /*player_->idle();*/ }

			if (IsKeyDown(KEY_KEY_A))		{ player_->setXDir(player_->getPlayerSpeed()); }
			else if(IsKeyDown(KEY_KEY_D))	{ player_->setXDir(-player_->getPlayerSpeed()); }
			else							{ player_->setXDir(0.0f); /*player_->idle();*/}

			if(IsKeyUp(KEY_SPACE)&&IsKeyUp(KEY_KEY_W)&&IsKeyUp(KEY_KEY_S)&&IsKeyUp(KEY_KEY_A)&&IsKeyUp(KEY_KEY_D)){player_->idle();}
		
		}
		

		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_ESCAPE && 
			event.KeyInput.PressedDown == true)
		{//handle the ESC key to display the pause menu
			if(pauseMenu || console.isVisible())
			{
				pauseMenu = false;
				console.setVisible(false);
			}
		
			else
				pauseMenu = true;

			engine_->play2D("sounds/button-20.wav",false);
		
		}
		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_F && 
			event.KeyInput.PressedDown == true && !console.isVisible() && !player_->isDead())
		{
			if(player_->getIsLamp())
				player_->setlamp(false);
			else
				player_->setlamp(true);
		}

		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_T && 
			event.KeyInput.PressedDown == true && !console.isVisible()&& !pauseMenu)
		{//handle f3 key to display the debugging information
			if (chatshown)
			{
				chatshown = false;

			}
			else
				chatshown = true;
			std::cout<<"T_Pressed!\n"<<"Value:"<<chatshown<<"\n";
			
		}

		if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_F3 && 
			event.KeyInput.PressedDown == true)
		{//handle f3 key to display the debugging information
			if(debug)
				debug = false;
			else
				debug = true;
		}


		

		if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == KEY_KEY_E &&
			event.KeyInput.PressedDown == true && !pauseMenu && !console.isVisible())
		{
			engine_->play2D("sounds/button-20.wav",false);
			if (inventory)
				inventory = false;
			else
			{
				inventory = true;
			}
		}

	    if (event.KeyInput.PressedDown)
		{
			if(event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key == IC_Console::IC_KEY_TILDE && !pauseMenu)
			{//handle the ~ key for the console
				if(!console.isVisible())
				{
					console.setVisible(true);
					
					return true;
				}
				else if(!event.KeyInput.Control)
				{
					console.setVisible(false);
					return true;
				}
				//KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
			}
			if(console.isVisible())
			{	

				console.handleKeyPress(event.KeyInput.Char, event.KeyInput.Key,event.KeyInput.Shift, event.KeyInput.Control);
				return true;
			}
		}
	}

	else if(event.EventType == irr::EET_LOG_TEXT_EVENT)
	{
		console.logMessage_ANSI(event.LogEvent.Level,event.LogEvent.Text);
		return true;
	}
	else if(event.EventType == irr::EET_MOUSE_INPUT_EVENT)
	{
		return console.isVisible();
	}
	
	if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = Context_.device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{

			case EGET_SCROLL_BAR_CHANGED:
				if (id == GUI_ID_TRANSPARENCY_SCROLL_BAR)
				{
					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					setSkinTransparency(pos, env->getSkin());
				}
				break;

			case EGET_BUTTON_CLICKED: //if one of our gui buttons is clicked handle it
				switch(id)
				{
				case GUI_ID_QUIT_BUTTON:
					Context_.device->closeDevice();
					engine_->play2D("sounds/button-25.wav",false);
					return true;

				case GUI_ID_START:

					engine_->play2D("sounds/button-25.wav",false);
					if(started)
						started = false;
					else
					{
						
						started = true;
						guienv_->clear(); //Clear out the main menu buttons when game starts
					}
					
					return true;

				case GUI_ID_CREDITS:
					{
						if (!creditsDisplayed)
						{
							creditsDisplayed = true;
						}
					}
					return true;

				case GUI_ID_GOTO_MAIN_MENU:
					started = false;
					pauseMenu = false;
					guienv_->clear();
					engine_->play2D("sounds/button-25.wav",false);
					return true;

				case GUI_ID_RESUME:
					if(pauseMenu)//this fixes the menu sound bug Mike
						engine_->play2D("sounds/button-25.wav",false);
					pauseMenu = false;
					
					return true;
					
				case GUI_ID_FILE_OPEN_BUTTON:
					Context_.listbox->addItem(L"File open");
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
					Context_.listbox->addItem(dialog->getFileName());
				}
				break;

			default:
				break;
			}
		}

	return false;
}


bool consoleevent::IsKeyDown(irr::EKEY_CODE keyCode) const
{
	return KeyIsDown[keyCode];
}

bool consoleevent::IsKeyUp(irr::EKEY_CODE keyCode) const
{
	return !KeyIsDown[keyCode];
}

void consoleevent::drawMainMenu()//draws the main menu for the game
{
	IGUISkin* skin = guienv_->getSkin();
	
	//add a button to our gui here
	
	ITexture* backgroundImage = driver_->getTexture("bill/creepyman-alt.jpg");
	ITexture* titleImage = driver_->getTexture("bill/logo.png");

	driver_->draw2DImage(backgroundImage,position2d<s32>(0,0),rect<s32>(0,0,800,600));
	driver_->draw2DImage(titleImage,position2d<s32>(0,0),rect<s32>(0,0,424,127),NULL,SColor(255,255,255,255), true);

	guienv_->addButton(rect<s32>(30,420,145,450), 0, GUI_ID_START,
		L"Start", L"Starts the game");
	guienv_->addButton(rect<s32>(30,460,145,490), 0, GUI_ID_OPTIONS,
		L"Options", L"Adjust game settings");
	guienv_->addButton(rect<s32>(30,500,145,530), 0, GUI_ID_CREDITS,
		L"Credits", L"Click to see who made the game");
	guienv_->addButton(rect<s32>(30,540,145,570), 0, GUI::GUI_ID_QUIT_BUTTON,
		L"Exit", L"Exit to the Desktop");


	//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
	if(font)
		skin->setFont(font);
	skin->setFont(guienv_->getBuiltInFont(),EGDF_TOOLTIP);
	device_->getCursorControl()->setVisible(true);
	device_->getCursorControl()->setPosition(device_->getCursorControl()->getPosition());
	guienv_->drawAll();
}




void consoleevent::drawPauseMenu ()// render the pause menu 
{
	if (inventory)
		inventory = false;
	//guienv_->drawAll();

	IGUISkin* skin = guienv_->getSkin();
	//guienv_ = device_->getGUIEnvironment();
	//guienv_->drawAll();
	//add a button to our gui here	
	driver_->draw2DRectangle(SColor(150,0,0,0),rect<s32>(320,150,500,450));

	ITexture* titleImage = driver_->getTexture("bill/pauselogo.png");
	driver_->draw2DImage(titleImage,position2d<s32>(290,150),rect<s32>(0,0,240,38),NULL,SColor(255,255,255,255), true);

	guienv_->addButton(rect<s32>(350,285,470,315), 0, GUI_ID_RESUME,
		L"Resume", L"Unpauses the game");
	guienv_->addButton(rect<s32>(350,325,470,355), 0, GUI_ID_PAUSE_OPTIONS,
		L"Options", L"Set up game things");
	guienv_->addButton(rect<s32>(350,365,470,395), 0, GUI_ID_GOTO_MAIN_MENU,
		L"Main Menu", L"Goes back to the main menu");
	guienv_->addButton(rect<s32>(350,405,470,435), 0, GUI::GUI_ID_QUIT_BUTTON,
		L"Quit", L"Exits Program");

		//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
		if(font)
			skin->setFont(font);
	skin->setFont(guienv_->getBuiltInFont(),EGDF_TOOLTIP);
	device_->getCursorControl()->setVisible(true);
	device_->getCursorControl()->setPosition(device_->getCursorControl()->getPosition());

	guienv_->drawAll();

}

void consoleevent::drawInventory ()
{
	const int menuWidth = 700; 

	const int clueBoxSpace = 10;
	const int clueBoxSize = 50;

	int boxVertPos = 300;
	int boxHorzPos = 80;

	driver_->draw2DRectangle(SColor(150,0,0,0),rect<s32>(50,50, 50+menuWidth,500));

	driver_->draw2DRectangle(SColor(255, 30, 30, 30),rect<s32>(70,70, 270,480));

	//This loop draws three boxes for the players equipped items
	for (int i = 0; i < 3; ++i)
	{
		driver_->draw2DRectangle(SColor(255, 80, 80, 80),rect<s32>(boxHorzPos,boxVertPos, boxHorzPos+clueBoxSize, boxVertPos+clueBoxSize));
		if (player_->getFoundClues().size() > i)
			driver_->draw2DRectangle(SColor(255,200,50,50),rect<s32>(boxHorzPos+CLUE_DRAW_OFFSET,boxVertPos+CLUE_DRAW_OFFSET, 
															boxHorzPos+clueBoxSize-CLUE_DRAW_OFFSET, boxVertPos+clueBoxSize-CLUE_DRAW_OFFSET));
		boxVertPos = (boxVertPos+clueBoxSize) + clueBoxSpace;
		boxHorzPos = (boxHorzPos+clueBoxSize) + clueBoxSpace;
	}

	// This loop draws all of the positions for the inventory items
	boxVertPos = 180;
	boxHorzPos = 280;
	for (int i = 0; i < 24; ++i)
	{
		driver_->draw2DRectangle(SColor(255, 80, 80, 80),rect<s32>(boxHorzPos,boxVertPos, boxHorzPos+clueBoxSize, boxVertPos+clueBoxSize));
		boxHorzPos = (boxHorzPos+clueBoxSize) + clueBoxSpace;

		if (boxHorzPos > menuWidth)
		{
			boxHorzPos = 280;
			boxVertPos = (boxVertPos+clueBoxSize) + clueBoxSpace;
		}
	}

	ITexture* invTitle = driver_->getTexture("bill/inv.png");
	driver_->draw2DImage(invTitle,position2d<s32>(290,60),rect<s32>(0,0,303,72),NULL,SColor(255,255,255,255), true);

	ITexture* character = driver_->getTexture("bill/unknown.jpg");
	driver_->draw2DImage(character,position2d<s32>(80,80),rect<s32>(0,0,180,150),NULL,SColor(255,255,255,255), true);

	gui::IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/bigfont.png");
	font->draw("Unknown", rect<s32>(80,230, 180,250), SColor(255, 255, 255, 200));

	device_->getCursorControl()->setVisible(true);
	device_->getCursorControl()->setPosition(device_->getCursorControl()->getPosition());
}

void consoleevent::playerNpcCollisionCheck()
{
	//player_->getPSceneNode()->
	/*
	for(int i=0; i < world_->getNumManifolds(); i++)
	{
		ICollisionCallbackInformation *info = world_->getCollisionCallback(i);

		bool hasCollideAttribute = (info->getBody0()->getAttributes()->existsAttribute("collide"));
		ICollisionObject* obj = (hasCollideAttribute) ? info->getBody0() : info->getBody1();

		if(obj->getAttributes()->existsAttribute("collide") && obj->getAttributes()->getAttributeAsBool("collide") == true)
		{
			obj->getAttributes()->setAttribute("collide", false);
			world_->addToDeletionQueue(obj);
		}

		obj = (hasCollideAttribute) ? info->getBody1() : info->getBody0();
		if(obj->getAttributes()->existsAttribute("vehicle"))
		{
			if(obj->getAttributes()->existsAttribute("destroyable"))
			{
					// Since we set a reference to the raycast vehicle of each tank, it will be removed automatically
					// by irrBullet when the object is removed!
					world_->removeCollisionObject(obj);				
			}
		}

		delete info;
	}
	
	//player_->getKinematicChar()->getGhostObject()->setCompanionId(0);
	//npc_.character_->GhostObject->setCompanionId(1);

	if (npc_.getKinematicChar()->getGhostObject()->checkCollideWith(player_->getKinematicChar()->getGhostObject()))
	{
		npc_.isHit() = true;
		std::cout << npc_.getHealth() << std::endl;
	}
	else
		npc_.isHit() = false;
		*/
}

void consoleevent::addClueObject(ClueObject* object)
{ 
	if(object->getObjectType() == TYPE_CLUE)
	{
		irr::core::string<char> nameEdit = object->getModel()->getName();
		nameEdit += clueObjects_.size();
		object->getModel()->setName(nameEdit);

		clueObjects_.push_back(object);
	}
	
}

remotePlayer* consoleevent::findRemotePlayer (std::string nodeName)
{
	//std::map<std::string,remotePlayer> rms = client_->getNetworkData()->getRemotePlayers();
	std::map<std::string,remotePlayer>::iterator it = client_->getNetworkData()->getRemotePlayers().begin();
	remotePlayer* rm = &it->second;
	while (rm->getPSceneNode()->getName() != nodeName && it != client_->getNetworkData()->getRemotePlayers().end())
	{
		rm = &it->second;
		++it;
	}

	return rm;
}

void consoleevent::update(u32 then, u32 now)//update the game throught the gameloop
{
	gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();

	if (!player_->isDead())
	{
		if ((player_->getXDir() != 0.0f || player_->getZDir() != 0.0f) && player_->isOnGround())
		{
			player_->animate(EMAT_STAND);
			player_->setStepSoundPaused(false);
		}
		else
		{
			player_->animate(EMAT_RUN);
			player_->setStepSoundPaused(true);
		}
	}	

	if (player_->isKillerKnown() && npc_->isLoaded() == NOT_LOADED)
		npc_->isLoaded() = LOADING;
	else if (npc_->isLoaded() == LOADING)
		npc_->loadModel();

	if(chatshown)
		drawMessage(L"T Was Pressed!");
	if(debug)
	{
		gui_->drawDebug(font ,driver_,player_->getPosition());
		world_->debugDrawWorld(true);
	}
	if(pauseMenu)drawPauseMenu();
		
//	if(chatshown){gui_->drawchat(guienv_);}
	
	if(inventory) drawInventory();
	if(!started)
		drawMainMenu();//guienv_->drawAll();

	

	
}

void consoleevent::drawMessage(wchar_t* message)
{
	gui_->addDialogBox(message);
	
}
