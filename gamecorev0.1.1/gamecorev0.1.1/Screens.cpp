#include "Screens.h"
#include "player.h"
#include "GameFlow.h"
#include "ClueObject.h"
#include "UiAnchor.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace gui;
using namespace scene;

namespace
{
	void removeButtons(IGUIEnvironment* guienv, const s32 (&ids)[4])
	{
		for (int i = 0; i < 4; ++i)
			if (IGUIElement* button = guienv->getRootGUIElement()->getElementFromId(ids[i], true))
				button->remove();
	}

	const s32 mainMenuButtons[] = { GameFlow::GUI_ID_START, GameFlow::GUI_ID_OPTIONS, GameFlow::GUI_ID_CREDITS, GameFlow::GUI_ID_QUIT_BUTTON };
	const s32 pauseMenuButtons[] = { GameFlow::GUI_ID_RESUME, GameFlow::GUI_ID_PAUSE_OPTIONS, GameFlow::GUI_ID_GOTO_MAIN_MENU, GameFlow::GUI_ID_QUIT_BUTTON };
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
	dimension2d<u32> screen = driver->getScreenSize();

	SColor healthColor;
	//draw health color based on how much health they have
	if(thePlayer.getHealth() >=60 && thePlayer.getHealth() < 101){healthColor = green;}
	else if(thePlayer.getHealth() >=30 && thePlayer.getHealth() < 60){healthColor = yellow;}
	else if(thePlayer.getHealth() >=1 && thePlayer.getHealth() < 30){healthColor = red;}
	else healthColor = blue;

	driver->draw2DRectangle(SColor(150,0,0,0), anchored(rect<s32>(290,530, 560,570), 0.5f, 1.0f, screen));
	driver->draw2DRectangle(healthColor, anchored(rect<s32>(300,540, 300+core::clamp(thePlayer.getHealth(),0,100)*5/2,560), 0.5f, 1.0f, screen));

	//draw the health bar
	driver->draw2DRectangle(SColor(150,0,0,0), anchored(rect<s32>(660,458, 760,570), 1.0f, 1.0f, screen));

	//draw the current weapon
	ITexture* currentWeapon = driver->getTexture("bill/icons/hand-small.jpg");
	driver->draw2DImage(currentWeapon, anchored(rect<s32>(670,470,750,560), 1.0f, 1.0f, screen).UpperLeftCorner,rect<s32>(0,0,80,90));

	IGUIFont* font2 = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	core::stringw fpsstr;//draw the current hunger and water levels
	fpsstr += "Water:";
	
	font2->draw(fpsstr,//draw current fps for debugging
		anchored(rect<s32>(370,480, 470 ,560), 0.5f, 1.0f, screen),white);//set up the color

	SColor color;
	//draw health color based on how much health they have
	if(thePlayer.getWater() >=60 && thePlayer.getWater() < 101){color = blue;}
	else if(thePlayer.getWater() >=30 && thePlayer.getWater() < 60){color = yellow;}
	else if(thePlayer.getWater() >=1 && thePlayer.getWater() < 30){color = red;}
	fpsstr="";
	fpsstr += thePlayer.getWater();
	font2->draw(fpsstr,//draw current fps for debugging
		anchored(rect<s32>(410,480, 510 ,560), 0.5f, 1.0f, screen),color);//set up the color
}

void MainMenuScreen::draw()//draws the main menu for the game
{
	IGUIEnvironment* guienv = device_->getGUIEnvironment();
	IVideoDriver* driver = device_->getVideoDriver();
	IGUISkin* skin = guienv->getSkin();
	dimension2d<u32> screen = driver->getScreenSize();
	
	//add a button to our gui here
	
	ITexture* backgroundImage = driver->getTexture("bill/theuninvitedlogo.jpg");
	//ITexture* titleImage = driver->getTexture("bill/logo.png");

	driver->draw2DImage(backgroundImage,letterboxed(screen),rect<s32>(0,0,800,600));
	//driver->draw2DImage(titleImage,position2d<s32>(0,0),rect<s32>(0,0,424,127),NULL,SColor(255,255,255,255), true);

	if (!guienv->getRootGUIElement()->getElementFromId(GameFlow::GUI_ID_CREDITS, true))
	{
		guienv->addButton(anchored(rect<s32>(330,200,450,240), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_START,
			L"Start", L"Starts the game");
		guienv->addButton(anchored(rect<s32>(330,260,450,300), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_OPTIONS,
			L"Options", L"Adjust game settings");
		guienv->addButton(anchored(rect<s32>(330,320,450,360), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_CREDITS,
			L"Credits", L"Click to see who made the game");
		guienv->addButton(anchored(rect<s32>(330,380,450,420), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_QUIT_BUTTON,
			L"Exit", L"Exit to the Desktop");
	}


	//2 types of gui fonts
	gui::IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	if(font)
		skin->setFont(font);
	
	font->draw("v0.1.1 (alpha)",anchored(rect<s32>(20,570,20,590), 0.0f, 1.0f, screen),SColor(255,255,255,255));

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
	dimension2d<u32> screen = driver->getScreenSize();
	//guienv = device_->getGUIEnvironment();
	//guienv->drawAll();
	//add a button to our gui here	
	driver->draw2DRectangle(SColor(150,0,0,0),anchored(rect<s32>(320,150,500,450), 0.5f, 0.5f, screen));

	ITexture* titleImage = driver->getTexture("bill/pauselogo.png");
	driver->draw2DImage(titleImage,anchored(rect<s32>(290,150,530,188), 0.5f, 0.5f, screen).UpperLeftCorner,rect<s32>(0,0,240,38),NULL,SColor(255,255,255,255), true);

	if (!guienv->getRootGUIElement()->getElementFromId(GameFlow::GUI_ID_GOTO_MAIN_MENU, true))
	{
		guienv->addButton(anchored(rect<s32>(350,285,470,315), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_RESUME,
			L"Resume", L"Unpauses the game");
		guienv->addButton(anchored(rect<s32>(350,325,470,355), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_PAUSE_OPTIONS,
			L"Options", L"Set up game things");
		guienv->addButton(anchored(rect<s32>(350,365,470,395), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_GOTO_MAIN_MENU,
			L"Main Menu", L"Goes back to the main menu");
		guienv->addButton(anchored(rect<s32>(350,405,470,435), 0.5f, 0.5f, screen), 0, GameFlow::GUI_ID_QUIT_BUTTON,
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

void DeathScreen::draw()
{
	gui::IGUIFont* deathFont = device_->getGUIEnvironment()->getFont("bill/bigfont.png");
	dimension2d<u32> screen = device_->getVideoDriver()->getScreenSize();
	if (deathFont)
	{
		deathFont->draw(L"You died", rect<s32>(0, screen.Height/2 - 60, screen.Width, screen.Height/2), SColor(255,200,0,0), true, true);
		deathFont->draw(L"Press Esc for Main Menu", rect<s32>(0, screen.Height/2, screen.Width, screen.Height/2 + 40), SColor(255,255,255,255), true, true);
	}
}

void NoteScreen::draw(const ClueObject& clue)
{
	IVideoDriver* driver = device_->getVideoDriver();
	const string<char>& message = clue.getMessage();
	int width = 250;
	int height = 400;
	position2d<s32> corner = anchored(rect<s32>(500,100,500+width,100+height), 1.0f, 0.5f, driver->getScreenSize()).UpperLeftCorner;
	int posX = corner.X;
	int posY = corner.Y;

	int textHeigt = 50;
	int charWidth = 15;
	int textOffset = 40;

	IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/console.bmp");//device_->getGUIEnvironment()->getBuiltInFont();

	//clueProperties_.showClue_ = true;
	ITexture* note = driver->getTexture("textures/bloodyNote.png");
	
	driver->draw2DImage(note,position2d<s32>(posX,posY),rect<s32>(0,0,width,height),NULL,SColor(255,255,255,255), true);

	int lineStart = 0;
	int lineLength = 20;
	int i = lineLength;
	int j = 0;
	string<char> line;
	while (i < (int)message.size())
	{
		j = 0;	
		if (message[i] != ' ')
		{					
			while ((i+j < (int)message.size()) && message[i-j] != ' ')
				++j;
		}
		
		line = message.subString(lineStart,lineLength);
		lineStart += lineLength-j;
		i += lineLength;
		posY += textOffset;
		
		font->draw(line, 
				   rect<s32>(posX+textOffset, posY+textOffset, 
							 posX+(line.size()*charWidth), posY+textOffset+textHeigt),
				   SColor(255,0,0,0));
	}
	//Draw the last line, since the quits before it prints. 
	//Might want to find a better way to do this
	line = message.subString(lineStart+1, message.size()-lineStart);
	posY += textOffset;
	font->draw(line, 
		rect<s32>(posX+textOffset, posY+textOffset, 
		posX+(line.size()*charWidth), posY+textOffset+textHeigt),
		SColor(255,0,0,0));
}

void drawIntro(IrrlichtDevice* device)
{
	device->getVideoDriver()->beginScene(true, false, irr::video::SColor(255,0,0,0));

	ITexture* backgroundImage = device->getVideoDriver()->getTexture("bill/intro.jpg");

	device->getVideoDriver()->draw2DImage(backgroundImage,letterboxed(device->getVideoDriver()->getScreenSize()),rect<s32>(0,0,800,600));
	device->getGUIEnvironment()->drawAll();
	device->getVideoDriver()->endScene();


}

void drawLoading(IrrlichtDevice* device)
{
	IVideoDriver* driver = device->getVideoDriver();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	driver->beginScene(true, false, irr::video::SColor(255,0,0,0));

	driver->getTexture("bill/creepyman.jpg");
	driver->getTexture("bill/loading.png");

	//driver->draw2DImage(backgroundImage,position2d<s32>(0,0),rect<s32>(0,0,800,600));
	//driver->draw2DImage(titleImage,position2d<s32>(100,100),rect<s32>(40,40,600,300),NULL,SColor(255,255,255,255), false);

	guienv->drawAll();



	irr::gui::IGUIFont* font = guienv->getFont("bill/bigfont.png");

	font->draw(L"LOADING...", anchored(irr::core::rect<irr::s32>(325,275,475,325), 0.5f, 0.5f, driver->getScreenSize()), irr::video::SColor(255,255,255,255));

	driver->endScene();


}
