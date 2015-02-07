/************************************************************************/
/*
    consoleevent.h
	Purpose: handles all events in the game
	By:Michael Carey && Dan Brown

*/
/************************************************************************/

#pragma once
#include<irrlicht.h>
#include<irrbullet.h>
#include <irrKlang.h>
#include "console.h"
#include "SAppcontext.h"
#include"GUI.h"
#include "player.h"

#include <iostream>


class consoleevent : public irr::IEventReceiver, public GUI
{
public:
	enum{ESC = 101, GUI_ID_START = 0, GUI_ID_OPTIONS, GUI_ID_CREDITS, GUI_ID_RESUME, GUI_ID_PAUSE_OPTIONS, GUI_ID_GOTO_MAIN_MENU};

	consoleevent(irr::IrrlichtDevice * device, irr::gui::IGUIEnvironment* guienv,
		irr::video::IVideoDriver* driver,SAppContext & Context,GUI gui, 
		/*irr::scene::ICameraSceneNode* camera,*/ irrklang::ISoundEngine* engine, player* thePlayer, irrBulletWorld* world);
	
	~consoleevent();

	bool OnEvent(const irr::SEvent& event);

	bool isvis(){return console.isVisible();}

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

	void drawInventory ();

	void update(irr::u32 then, irr::u32 now);

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
	GUI gui_;
	player* player_;
	irrBulletWorld* world_;
	bool debug;
	bool pauseMenu;
	bool started;
	bool chatshown;
	bool creditsDisplayed;
	bool inventory;
	irrklang::ISoundEngine* engine_;
	// We use this array to store the current state of each key
	bool KeyIsDown[irr::KEY_KEY_CODES_COUNT];
};