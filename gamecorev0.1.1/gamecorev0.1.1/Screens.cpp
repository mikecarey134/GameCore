#include "Screens.h"
#include "player.h"
#include "GameFlow.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace gui;

namespace
{
	void removeButtons(IGUIEnvironment* guienv, const s32 (&ids)[4])
	{
		for (int i = 0; i < 4; ++i)
			if (IGUIElement* button = guienv->getRootGUIElement()->getElementFromId(ids[i], true))
				button->remove();
	}

	const s32 mainMenuButtons[] = { GameFlow::GUI_ID_START, GameFlow::GUI_ID_OPTIONS, GameFlow::GUI_ID_CREDITS, GUI::GUI_ID_QUIT_BUTTON };
	const s32 pauseMenuButtons[] = { GameFlow::GUI_ID_RESUME, GameFlow::GUI_ID_PAUSE_OPTIONS, GameFlow::GUI_ID_GOTO_MAIN_MENU, GUI::GUI_ID_QUIT_BUTTON };
}

namespace
{
	const SColor red(255,255,0,0);
	const SColor blue(255,0,0,255);
	const SColor yellow(255,255,252,66);
	const SColor green(255,0,255,0);
	const SColor white(255,255,255,255);
}

void Hud::draw(const player& thePlayer)
{
	IVideoDriver* driver = device_->getVideoDriver();

	SColor healthColor;
	//draw health color based on how much health they have
	if(thePlayer.getHealth() >=60 && thePlayer.getHealth() < 101){healthColor = green;}
	else if(thePlayer.getHealth() >=30 && thePlayer.getHealth() < 60){healthColor = yellow;}
	else if(thePlayer.getHealth() >=1 && thePlayer.getHealth() < 30){healthColor = red;}
	else healthColor = blue;

	driver->draw2DRectangle(SColor(150,0,0,0), rect<s32>(290,530, 560,570));
	driver->draw2DRectangle(healthColor, rect<s32>(300,540, 300+core::clamp(thePlayer.getHealth(),0,100)*5/2,560));

	//draw the health bar
	driver->draw2DRectangle(SColor(150,0,0,0), rect<s32>(660,458, 760,570));

	//draw the current weapon
	ITexture* currentWeapon = driver->getTexture("bill/icons/hand-small.jpg");
	driver->draw2DImage(currentWeapon, position2d<s32>(670,470),rect<s32>(0,0,80,90));

	IGUIFont* font2 = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	core::stringw fpsstr;//draw the current hunger and water levels
	fpsstr += "Water:";
	
	font2->draw(fpsstr,//draw current fps for debugging
		rect<s32>(370,480, 470 ,560),white);//set up the color

	SColor color;
	//draw health color based on how much health they have
	if(thePlayer.getWater() >=60 && thePlayer.getWater() < 101){color = blue;}
	else if(thePlayer.getWater() >=30 && thePlayer.getWater() < 60){color = yellow;}
	else if(thePlayer.getWater() >=1 && thePlayer.getWater() < 30){color = red;}
	fpsstr="";
	fpsstr += thePlayer.getWater();
	font2->draw(fpsstr,//draw current fps for debugging
		rect<s32>(410,480, 510 ,560),color);//set up the color
}

void MainMenuScreen::draw()//draws the main menu for the game
{
	IGUIEnvironment* guienv = device_->getGUIEnvironment();
	IVideoDriver* driver = device_->getVideoDriver();
	IGUISkin* skin = guienv->getSkin();
	
	//add a button to our gui here
	
	ITexture* backgroundImage = driver->getTexture("bill/theuninvitedlogo.jpg");
	//ITexture* titleImage = driver->getTexture("bill/logo.png");

	driver->draw2DImage(backgroundImage,position2d<s32>(0,0),rect<s32>(0,0,800,600));
	//driver->draw2DImage(titleImage,position2d<s32>(0,0),rect<s32>(0,0,424,127),NULL,SColor(255,255,255,255), true);

	if (!guienv->getRootGUIElement()->getElementFromId(GameFlow::GUI_ID_CREDITS, true))
	{
		guienv->addButton(rect<s32>(330,200,450,240), 0, GameFlow::GUI_ID_START,
			L"Start", L"Starts the game");
		guienv->addButton(rect<s32>(330,260,450,300), 0, GameFlow::GUI_ID_OPTIONS,
			L"Options", L"Adjust game settings");
		guienv->addButton(rect<s32>(330,320,450,360), 0, GameFlow::GUI_ID_CREDITS,
			L"Credits", L"Click to see who made the game");
		guienv->addButton(rect<s32>(330,380,450,420), 0, GUI::GUI_ID_QUIT_BUTTON,
			L"Exit", L"Exit to the Desktop");
	}


	//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	if(font)
		skin->setFont(font);
	
	font->draw("v0.1.1 (alpha)",rect<s32>(20,570,20,590),SColor(255,255,255,255));

	guienv->drawAll();
	
}

void MainMenuScreen::hide()
{
	removeButtons(device_->getGUIEnvironment(), mainMenuButtons);
}

void PauseScreen::draw()// render the pause menu 
{
	IGUIEnvironment* guienv = device_->getGUIEnvironment();
	IVideoDriver* driver = device_->getVideoDriver();
	//guienv->drawAll();

	IGUISkin* skin = guienv->getSkin();
	//guienv = device_->getGUIEnvironment();
	//guienv->drawAll();
	//add a button to our gui here	
	driver->draw2DRectangle(SColor(150,0,0,0),rect<s32>(320,150,500,450));

	ITexture* titleImage = driver->getTexture("bill/pauselogo.png");
	driver->draw2DImage(titleImage,position2d<s32>(290,150),rect<s32>(0,0,240,38),NULL,SColor(255,255,255,255), true);

	if (!guienv->getRootGUIElement()->getElementFromId(GameFlow::GUI_ID_GOTO_MAIN_MENU, true))
	{
		guienv->addButton(rect<s32>(350,285,470,315), 0, GameFlow::GUI_ID_RESUME,
			L"Resume", L"Unpauses the game");
		guienv->addButton(rect<s32>(350,325,470,355), 0, GameFlow::GUI_ID_PAUSE_OPTIONS,
			L"Options", L"Set up game things");
		guienv->addButton(rect<s32>(350,365,470,395), 0, GameFlow::GUI_ID_GOTO_MAIN_MENU,
			L"Main Menu", L"Goes back to the main menu");
		guienv->addButton(rect<s32>(350,405,470,435), 0, GUI::GUI_ID_QUIT_BUTTON,
			L"Quit", L"Exits Program");
	}

		//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
		if(font)
			skin->setFont(font);
	skin->setFont(guienv->getBuiltInFont(),EGDF_TOOLTIP);

	guienv->drawAll();

}

void PauseScreen::hide()
{
	removeButtons(device_->getGUIEnvironment(), pauseMenuButtons);
}
