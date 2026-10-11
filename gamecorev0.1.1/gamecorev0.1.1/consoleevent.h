/************************************************************************/
/*
    consoleevent.h
	Purpose: handles all events in the game
	By:Michael Carey && Dan Brown

*/
/************************************************************************/

#pragma once
#include <fstream>
#include <irrlicht.h>
#include <irrbullet.h>
#include "SoundEngine.h"
#include <list>
#include "console.h"
#include "SAppcontext.h"
#include "GUI.h"
#include "player.h"
#include "NPC.h"
#include "InputRouter.h"
#include "GameMap.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "InternalServer.h"


#include <iostream>

#define CLUE_DRAW_OFFSET 10
class InventoryScreen;


class consoleevent : public InputLayer, public GUI
{
public:
	friend class NPC;
	friend class Player;

	enum{GUI_ID_START = 0, GUI_ID_OPTIONS, GUI_ID_CREDITS, GUI_ID_RESUME, GUI_ID_PAUSE_OPTIONS, GUI_ID_GOTO_MAIN_MENU};

	consoleevent(irr::IrrlichtDevice * device, irr::gui::IGUIEnvironment* guienv,
		irr::video::IVideoDriver* driver,SAppContext & Context,GUI* gui, 
		/*irr::scene::ICameraSceneNode* camera,*/ gamecore::SoundEngine* engine, 
		player* thePlayer, irrBulletWorld* world, NPC* npc, GameMap* currentMap, InternalServer* client);
	
	~consoleevent();

	bool onEvent(const irr::SEvent& event) override;
	void setInputState(const InputState* input) { input_ = input; }

	bool isConsoleVisible(){return console.isVisible();}
	IC_Console& getConsole() { return console; }

	bool isPaused()		{ return (pauseMenu || inventory || chatshown); }

	virtual bool IsKeyDown(irr::EKEY_CODE keyCode) const;

	virtual bool IsKeyUp(irr::EKEY_CODE keyCode) const;

	bool getStarted()      { return started; }
	bool getPaused()       { return pauseMenu; }
	bool getIsInventory()  { return inventory; }
	void setIsInventory(bool open) { inventory = open; }
	InventoryScreen* getInventoryScreen() { return inventoryScreen_; }
	std::list<ClueObject*>& getClueObjects() { return clueObjects_; }
	bool getIsConsole()    {return console.isVisible();}

	void displayconsole(const u32 dm){console.renderConsole(guienv_,driver_,dm);}
	void drawIntro     ();
	void drawMainMenu  ();

	void drawPauseMenu ();
	void drawMessage   (const wchar_t* message);
	void newRound      ();
	void goToMainMenu  ();
	void drawInventory ();
	void setInventoryScreen(InventoryScreen* screen) { inventoryScreen_ = screen; }

	void playerNpcCollisionCheck();

	void addClueObject(ClueObject* object); 

	remotePlayer* findRemotePlayer(std::string nodeName);

	void update(irr::u32 then, irr::u32 now);
	
	void clear_console(){console.clearMessages();}
	
	struct SMouseState
	{
		core::position2di Position;
		bool LeftButtonDown;
		bool RightButtonDown;
		SMouseState() : LeftButtonDown(false),RightButtonDown(false) { }
	} MouseState;

private:

	IC_Console console;
	irr::gui::IGUIEnvironment* guienv_;
	irr::IrrlichtDevice * device_;
	irr::video::IVideoDriver* driver_;
	SAppContext & Context_;
	GUI* gui_;
	player* player_;
	//std::list<remotePlayer*> remotePlayers_;
	NPC* npc_;
	irrBulletWorld* world_;
	bool debug;
	bool pauseMenu;
	bool started;
	bool chatshown;
	bool creditsDisplayed;
	bool inventory;
	gamecore::SoundEngine* engine_;
	GameMap* currentMap_;
	InternalServer* client_;
	irr::gui::IGUIEditBox* ebox_;

	std::list<ClueObject*> clueObjects_; 

	// We use this array to store the current state of each key
	const InputState* input_;


	//debugging get rid of this at some point
	std::ofstream coords;
	InventoryScreen* inventoryScreen_;
};