/************************************************************************/
/*
    GameFlow.cpp
	Purpose: game state, menus and per-frame updates
	By:Michael Carey && Dan Brown

*/
/************************************************************************/


#include "GameFlow.h"
#include "defaultCmds.h"
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
							inventoryScreen_(nullptr), debugOverlay_(device, world), mainMenu_(device), pause_(device), death_(device), note_(device)
{
	//open up the file to get set up the coords
	//coords.open("coords.txt");
									//add world psychics to the console
	console.loadDefaultCommands(device);//load the default commands help echo etc
	registerGameCommands(console, thePlayer, world, device, currentMap_);
	console.initializeConsole(guienv, dimension2d<s32>(800,500));//prepare the console
	console.setVisible(false);




}
GameFlow::~GameFlow(){ coords.close();}


void GameFlow::drawMainMenu()//draws the main menu for the game
{
	mainMenu_.draw();
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

void GameFlow::drawNotes()
{
	for (std::list<ClueObject*>::iterator clue = clueObjects_.begin(); clue != clueObjects_.end(); ++clue)
		if ((*clue)->isNoteShowing())
			note_.draw(**clue);
}

void GameFlow::update(u32, u32)//update the game throught the gameloop
{
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

	if (pauseMenu && inventory)
	{
		inventory = false;
		if (inventoryScreen_)
			inventoryScreen_->close();
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
		death_.draw();

	if(chatshown)
		drawMessage(L"T Was Pressed!");
	if(debug)
	{
		debugOverlay_.draw(player_->getPosition());
	}
	if(pauseMenu)pause_.draw();
	else if(started)
		pause_.hide();
		
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
	pause_.hide();
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
		mainMenu_.hide(); //Clear out the main menu buttons when game starts
		newRound();
	}
}
