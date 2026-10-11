/************************************************************************/
/*
    GameFlow.cpp
	Purpose: game state, menus and per-frame updates
	By:Michael Carey && Dan Brown

*/
/************************************************************************/


#include "GameFlow.h"
#include "InventoryScreen.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace gamecore;




GameFlow::GameFlow(irr::IrrlichtDevice *device, irr::gui::IGUIEnvironment* guienv,
						   irr::video::IVideoDriver* driver, SAppContext & Context,GUI* gui,
						   gamecore::SoundEngine* engine, player* thePlayer, irrBulletWorld* world, 
						   NPC* npc, GameMap* currentMap, InternalServer* client):
							guienv_(guienv),device_(device),driver_(driver),Context_(Context),gui_(gui),player_(thePlayer),							
							npc_(npc), world_(world), debug(false),pauseMenu(false), started(false), chatshown(false), 
							creditsDisplayed(false), inventory(false), engine_(engine), currentMap_(currentMap), client_(client),
							inventoryScreen_(nullptr)
{
	//open up the file to get set up the coords
	//coords.open("coords.txt");
	console.addExtensions(thePlayer,world,device,currentMap_);//register the player in the console
									//add world psychics to the console
	console.loadDefaultCommands(device);//load the default commands help echo etc
	console.initializeConsole(guienv, dimension2d<s32>(800,500));//prepare the console
	console.setVisible(false);




}
GameFlow::~GameFlow(){ coords.close();}

static const s32 mainMenuButtons[] = { GameFlow::GUI_ID_START, GameFlow::GUI_ID_OPTIONS, GameFlow::GUI_ID_CREDITS, GUI::GUI_ID_QUIT_BUTTON };
static const s32 pauseMenuButtons[] = { GameFlow::GUI_ID_RESUME, GameFlow::GUI_ID_PAUSE_OPTIONS, GameFlow::GUI_ID_GOTO_MAIN_MENU, GUI::GUI_ID_QUIT_BUTTON };

static void removeButtons(IGUIEnvironment* guienv, const s32 (&ids)[4])
{
	for (int i = 0; i < 4; ++i)
		if (IGUIElement* button = guienv->getRootGUIElement()->getElementFromId(ids[i], true))
			button->remove();
}

void GameFlow::drawMainMenu()//draws the main menu for the game
{
	IGUISkin* skin = guienv_->getSkin();
	
	//add a button to our gui here
	
	ITexture* backgroundImage = driver_->getTexture("bill/theuninvitedlogo.jpg");
	//ITexture* titleImage = driver_->getTexture("bill/logo.png");

	driver_->draw2DImage(backgroundImage,position2d<s32>(0,0),rect<s32>(0,0,800,600));
	//driver_->draw2DImage(titleImage,position2d<s32>(0,0),rect<s32>(0,0,424,127),NULL,SColor(255,255,255,255), true);

	if (!guienv_->getRootGUIElement()->getElementFromId(GUI_ID_CREDITS, true))
	{
		guienv_->addButton(rect<s32>(330,200,450,240), 0, GUI_ID_START,
			L"Start", L"Starts the game");
		guienv_->addButton(rect<s32>(330,260,450,300), 0, GUI_ID_OPTIONS,
			L"Options", L"Adjust game settings");
		guienv_->addButton(rect<s32>(330,320,450,360), 0, GUI_ID_CREDITS,
			L"Credits", L"Click to see who made the game");
		guienv_->addButton(rect<s32>(330,380,450,420), 0, GUI::GUI_ID_QUIT_BUTTON,
			L"Exit", L"Exit to the Desktop");
	}


	//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	if(font)
		skin->setFont(font);
	
	font->draw("v0.1.1 (alpha)",rect<s32>(20,570,20,590),SColor(255,255,255,255));

	guienv_->drawAll();
	
}




void GameFlow::drawPauseMenu ()// render the pause menu 
{
	if (inventory)
	{
		inventory = false;
		if (inventoryScreen_)
			inventoryScreen_->close();
	}
	//guienv_->drawAll();

	IGUISkin* skin = guienv_->getSkin();
	//guienv_ = device_->getGUIEnvironment();
	//guienv_->drawAll();
	//add a button to our gui here	
	driver_->draw2DRectangle(SColor(150,0,0,0),rect<s32>(320,150,500,450));

	ITexture* titleImage = driver_->getTexture("bill/pauselogo.png");
	driver_->draw2DImage(titleImage,position2d<s32>(290,150),rect<s32>(0,0,240,38),NULL,SColor(255,255,255,255), true);

	if (!guienv_->getRootGUIElement()->getElementFromId(GUI_ID_GOTO_MAIN_MENU, true))
	{
		guienv_->addButton(rect<s32>(350,285,470,315), 0, GUI_ID_RESUME,
			L"Resume", L"Unpauses the game");
		guienv_->addButton(rect<s32>(350,325,470,355), 0, GUI_ID_PAUSE_OPTIONS,
			L"Options", L"Set up game things");
		guienv_->addButton(rect<s32>(350,365,470,395), 0, GUI_ID_GOTO_MAIN_MENU,
			L"Main Menu", L"Goes back to the main menu");
		guienv_->addButton(rect<s32>(350,405,470,435), 0, GUI::GUI_ID_QUIT_BUTTON,
			L"Quit", L"Exits Program");
	}

		//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
		if(font)
			skin->setFont(font);
	skin->setFont(guienv_->getBuiltInFont(),EGDF_TOOLTIP);

	guienv_->drawAll();

}

void GameFlow::drawInventory ()
{
	if (inventoryScreen_)
		inventoryScreen_->draw();
}

void GameFlow::playerNpcCollisionCheck()
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

void GameFlow::addClueObject(ClueObject* object)
{ 
	if(object->getObjectType() == TYPE_CLUE)
	{
		irr::core::string<char> nameEdit = object->getModel()->getName();
		nameEdit += (unsigned int)clueObjects_.size();
		object->getModel()->setName(nameEdit);

		clueObjects_.push_back(object);
	}
	
}

remotePlayer* GameFlow::findRemotePlayer (std::string nodeName)
{
	//std::map<std::string,remotePlayer> rms = client_->getNetworkData()->getRemotePlayers();
	std::map<std::string,remotePlayer>::iterator it = client_->getNetworkData()->getRemotePlayers().begin();
	remotePlayer* rm = &it->second;

	std::string rmName;
	if (!rm->isDead())
		rmName = rm->getPSceneNode()->getName();

	while (rmName != nodeName && it != client_->getNetworkData()->getRemotePlayers().end())
	{
		rm = &it->second;
		if (!rm->isDead())
			rmName = rm->getPSceneNode()->getName();

		++it;
	}

	return rm;
}

void GameFlow::update(u32, u32)//update the game throught the gameloop
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

	std::vector<vector3df> npcSpawns = currentMap_->getNpcLocations();
	if (player_->isKillerKnown() && npc_->isLoaded() == NOT_LOADED && !npcSpawns.empty())
		npc_->isLoaded() = LOADING;
	else if (npc_->isLoaded() == LOADING)
	{
		if (npcSpawns.empty())
			npc_->isLoaded() = NOT_LOADED;
		else
			npc_->loadModel(npcSpawns[rand() % npcSpawns.size()]);
	}

	if (started && player_->isDead() && !pauseMenu)
	{
		gui::IGUIFont* deathFont = device_->getGUIEnvironment()->getFont("bill/bigfont.png");
		dimension2d<u32> screen = driver_->getScreenSize();
		if (deathFont)
		{
			deathFont->draw(L"You died", rect<s32>(0, screen.Height/2 - 60, screen.Width, screen.Height/2), SColor(255,200,0,0), true, true);
			deathFont->draw(L"Press Esc for Main Menu", rect<s32>(0, screen.Height/2, screen.Width, screen.Height/2 + 40), SColor(255,255,255,255), true, true);
		}
	}

	if(chatshown)
		drawMessage(L"T Was Pressed!");
	if(debug)
	{
		gui_->drawDebug(font ,driver_,player_->getPosition());
		world_->debugDrawWorld(true);
	}
	if(pauseMenu)drawPauseMenu();
	else if(started)
		removeButtons(guienv_, pauseMenuButtons);
		
//	if(chatshown){gui_->drawchat(guienv_);}
	
	if(inventory) drawInventory();
	if(!started)
		drawMainMenu();//guienv_->drawAll();

	
	//add the coords to a log in order to get positions for grass nodes
	//coords << player_->getPosition().X << " " << player_->getPosition().Y << " " << player_->getPosition().Z << "\n" ;
	
}

void GameFlow::drawMessage(const wchar_t*)
{
	/*stringw msg;
	msg+= message;
	device_->getGUIEnvironment()->drawAll();
	//device_->getVideoDriver()->draw2DRectangle(SColor(150,0,0,0), rect<s32>(20,400, 400,550));
	ebox_ = device_->getGUIEnvironment()->addEditBox(L"",rect<s32>(10,400, 260,550),true,0,10);
	//gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();

	ebox_->getActiveFont();
	ebox_->setEnabled(true);
	ebox_->setMax(40);
	guienv_->setFocus(ebox_);*/
	
	

	//gui_->addDialogBox(message);
	
}

void GameFlow::newRound()
{
	std::vector<vector3df> spawns = currentMap_->getSpawnPoints();
	player_->respawn(spawns[rand() % spawns.size()]);
	for (std::list<ClueObject*>::iterator clue = clueObjects_.begin(); clue != clueObjects_.end(); ++clue)
		(*clue)->reset();
	npc_->reset();
}

void GameFlow::goToMainMenu()
{
	started = false;
	pauseMenu = false;
	removeButtons(guienv_, pauseMenuButtons);
	engine_->stopAllSounds();
	engine_->play2D("sounds/button-25.wav",false);
}

void GameFlow::startPressed()
{
	engine_->play2D("sounds/button-25.wav",false);
	if(started)
		started = false;
	else
	{
		started = true;
		removeButtons(guienv_, mainMenuButtons); //Clear out the main menu buttons when game starts
		newRound();
	}
}
