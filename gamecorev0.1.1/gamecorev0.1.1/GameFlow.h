/************************************************************************/
/*
    GameFlow.h
	Purpose: game state, menus and per-frame updates
	By:Michael Carey && Dan Brown

*/
/************************************************************************/

#pragma once
#include <fstream>
#include <irrlicht.h>
#include "SoundEngine.h"
#include <list>
#include "console.h"
#include "SAppcontext.h"
#include "player.h"
#include "NPC.h"
#include "GameMap.h"
#include "DebugOverlay.h"
#include "Screens.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include "InternalServer.h"


#include <iostream>

#define CLUE_DRAW_OFFSET 10
class InventoryScreen;


class GameFlow
{
public:
	friend class NPC;
	friend class Player;

	enum{GUI_ID_START = 0, GUI_ID_OPTIONS, GUI_ID_CREDITS, GUI_ID_RESUME, GUI_ID_PAUSE_OPTIONS, GUI_ID_GOTO_MAIN_MENU, GUI_ID_QUIT_BUTTON};

	GameFlow(irr::IrrlichtDevice * device, Canvas& canvas, irr::gui::IGUIEnvironment* guienv,
		irr::video::IVideoDriver* driver,SAppContext & Context,
		/*irr::scene::ICameraSceneNode* camera,*/ gamecore::SoundEngine* engine, 
		player* thePlayer, PhysicsWorld* physics, NPC* npc, GameMap* currentMap, InternalServer* client);
	
	~GameFlow();


	bool isConsoleVisible(){return console.isVisible();}
	IC_Console& getConsole() { return console; }

	bool isPaused()		{ return (pauseMenu || inventory || chatshown); }


	bool getStarted()      { return started; }
	bool getPaused()       { return pauseMenu; }
	bool getIsInventory()  { return inventory; }
	void setIsInventory(bool open) { inventory = open; }
	InventoryScreen* getInventoryScreen() { return inventoryScreen_; }
	std::list<ClueObject*>& getClueObjects() { return clueObjects_; }
	bool getIsConsole()    {return console.isVisible();}

	void displayconsole(const u32 dm){console.renderConsole(guienv_,driver_,dm);}
	void newRound      ();
	void goToMainMenu  ();
	void startPressed  ();
	void drawInventory ();
	void setInventoryScreen(InventoryScreen* screen) { inventoryScreen_ = screen; }


	void addClueObject(ClueObject* object); 

	remotePlayer* findRemotePlayer(std::string nodeName);

	void update(irr::u32 then, irr::u32 now);
	void draw();
	
	void clear_console(){console.clearMessages();}
	

private:
	friend class MenuLayer;
	friend class DebugLayer;
	friend class GameplayLayer;

	IC_Console console;
	irr::gui::IGUIEnvironment* guienv_;
	irr::IrrlichtDevice * device_;
	irr::video::IVideoDriver* driver_;
	SAppContext & Context_;
	player* player_;
	//std::list<remotePlayer*> remotePlayers_;
	NPC* npc_;
	bool debug;
	bool pauseMenu;
	bool started;
	bool chatshown;
	bool inventory;
	gamecore::SoundEngine* engine_;
	GameMap* currentMap_;
	InternalServer* client_;
	irr::gui::IGUIEditBox* ebox_;

	std::list<ClueObject*> clueObjects_; 

	// We use this array to store the current state of each key


	//debugging get rid of this at some point
	std::ofstream coords;
	InventoryScreen* inventoryScreen_;
	Hud hud_;
	DebugOverlay debugOverlay_;
	MainMenuScreen mainMenu_;
	PauseScreen pause_;
	DeathScreen death_;
	NoteScreen note_;
};