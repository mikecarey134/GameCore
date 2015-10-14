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
#include <irrKlang.h>
#include <list>
#include "console.h"
#include "SAppcontext.h"
#include "GUI.h"
#include "player.h"
#include "NPC.h"
#include "mapLoader.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "InternalServer.h"


#include <iostream>

#define CLUE_DRAW_OFFSET 10


class consoleevent : public irr::IEventReceiver, public GUI
{
public:
	friend class NPC;
	friend class Player;

	enum{ESC = 101, GUI_ID_START = 0, GUI_ID_OPTIONS, GUI_ID_CREDITS, GUI_ID_RESUME, GUI_ID_PAUSE_OPTIONS, GUI_ID_GOTO_MAIN_MENU};

	consoleevent(irr::IrrlichtDevice * device, irr::gui::IGUIEnvironment* guienv,
		irr::video::IVideoDriver* driver,SAppContext & Context,GUI* gui, 
		/*irr::scene::ICameraSceneNode* camera,*/ irrklang::ISoundEngine* engine, 
		player* thePlayer, irrBulletWorld* world, NPC* npc, mapLoader* currentMap, InternalServer* client);
	
	~consoleevent();

	bool OnEvent(const irr::SEvent& event);

	bool isConsoleVisible(){return console.isVisible();}

	bool isPaused()		{ return (pauseMenu || inventory || chatshown); }

	virtual bool IsKeyDown(irr::EKEY_CODE keyCode) const;

	virtual bool IsKeyUp(irr::EKEY_CODE keyCode) const;

	bool getStarted()      { return started; }
	bool getPaused()       { return pauseMenu; }
	bool getIsInventory()  { return inventory; }
	bool getIsConsole()    {return console.isVisible();}

	void displayconsole(const u32 dm){console.renderConsole(guienv_,driver_,dm);}
	void drawIntro     ();
	void drawMainMenu  ();

	void drawPauseMenu ();
	void drawMessage   (wchar_t* message);
	void drawInventory ();

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
	irrklang::ISoundEngine* engine_;
	mapLoader* currentMap_;
	InternalServer* client_;
	irr::gui::IGUIEditBox* ebox_;

	std::list<ClueObject*> clueObjects_; 

	// We use this array to store the current state of each key
	bool KeyIsDown[irr::KEY_KEY_CODES_COUNT];


	//debugging get rid of this at some point
	std::ofstream coords;
};