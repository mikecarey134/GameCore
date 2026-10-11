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
						   irr::video::IVideoDriver* driver, SAppContext & Context,
						   gamecore::SoundEngine* engine, player* thePlayer, PhysicsWorld* physics, 
						   NPC* npc, GameMap* currentMap, InternalServer* client):
							guienv_(guienv),device_(device),driver_(driver),Context_(Context),player_(thePlayer),							
							npc_(npc), debug(false),pauseMenu(false), started(false), chatshown(false), 
							inventory(false), engine_(engine), currentMap_(currentMap), client_(client),
							inventoryScreen_(nullptr), hud_(device), debugOverlay_(device, physics), mainMenu_(device), pause_(device), death_(device), note_(device)
{
	//open up the file to get set up the coords
	//coords.open("coords.txt");
									//add world psychics to the console
	console.loadDefaultCommands(device);//load the default commands help echo etc
	registerGameCommands(console, thePlayer, physics, device, currentMap_);
	dimension2d<u32> screen = driver->getScreenSize();
	console.getConfig().dimensionRatios.Y = 0.5f;
	console.initializeConsole(guienv, dimension2d<s32>((s32)screen.Width, (s32)screen.Height));//prepare the console
	console.setVisible(false);




}
GameFlow::~GameFlow(){ coords.close();}


void GameFlow::drawInventory ()
{
	if (inventoryScreen_)
		inventoryScreen_->draw();
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
	return client_->getNetworkData()->findByNode(nodeName);
}

void GameFlow::draw()
{
	if (!started)
	{
		mainMenu_.draw();
		return;
	}

	if (!inventory)
		hud_.draw(*player_);
	for (std::list<ClueObject*>::iterator clue = clueObjects_.begin(); clue != clueObjects_.end(); ++clue)
		if ((*clue)->isNoteShowing())
			note_.draw(**clue);
	if (player_->isDead() && !pauseMenu)
		death_.draw();
	if (inventory)
		drawInventory();
	if (pauseMenu)
		pause_.draw();
	else
		pause_.hide();
	if (debug)
		debugOverlay_.draw(player_->getPosition());
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

	//add the coords to a log in order to get positions for grass nodes
	//coords << player_->getPosition().X << " " << player_->getPosition().Y << " " << player_->getPosition().Z << "\n" ;
	
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
