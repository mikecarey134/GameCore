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
	dimension2d<u32> screen = canvas_.screenSize();

	SColor healthColor;
	//draw health color based on how much health they have
	if(thePlayer.getHealth() >=60 && thePlayer.getHealth() < 101){healthColor = green;}
	else if(thePlayer.getHealth() >=30 && thePlayer.getHealth() < 60){healthColor = yellow;}
	else if(thePlayer.getHealth() >=1 && thePlayer.getHealth() < 30){healthColor = red;}
	else healthColor = blue;

	canvas_.drawRect(anchored(rect<s32>(290,530, 560,570), 0.5f, 1.0f, screen), SColor(150,0,0,0));
	canvas_.drawRect(anchored(rect<s32>(300,540, 300+core::clamp(thePlayer.getHealth(),0,100)*5/2,560), 0.5f, 1.0f, screen), healthColor);

	//draw the health bar
	canvas_.drawRect(anchored(rect<s32>(660,458, 760,570), 1.0f, 1.0f, screen), SColor(150,0,0,0));

	//draw the current weapon
	TextureHandle currentWeapon = canvas_.loadTexture("bill/icons/hand-small.jpg");
	position2d<s32> weaponCorner = anchored(rect<s32>(670,470,750,560), 1.0f, 1.0f, screen).UpperLeftCorner;
	canvas_.drawImage(currentWeapon, rect<s32>(weaponCorner, dimension2d<s32>(80,90)), rect<s32>(0,0,80,90));

	FontHandle font2 = canvas_.loadFont("bill/console.bmp");
	core::stringw fpsstr;//draw the current hunger and water levels
	fpsstr += "Water:";
	
	canvas_.drawText(font2, fpsstr.c_str(), anchored(rect<s32>(370,480, 470 ,560), 0.5f, 1.0f, screen), white);

	SColor color;
	//draw health color based on how much health they have
	if(thePlayer.getWater() >=60 && thePlayer.getWater() < 101){color = blue;}
	else if(thePlayer.getWater() >=30 && thePlayer.getWater() < 60){color = yellow;}
	else if(thePlayer.getWater() >=1 && thePlayer.getWater() < 30){color = red;}
	fpsstr="";
	fpsstr += thePlayer.getWater();
	canvas_.drawText(font2, fpsstr.c_str(), anchored(rect<s32>(410,480, 510 ,560), 0.5f, 1.0f, screen), color);
}

void MainMenuScreen::draw()//draws the main menu for the game
{
	IGUIEnvironment* guienv = device_->getGUIEnvironment();
	IGUISkin* skin = guienv->getSkin();
	dimension2d<u32> screen = canvas_.screenSize();

	canvas_.drawImage(canvas_.loadTexture("bill/theuninvitedlogo.jpg"), letterboxed(screen), rect<s32>(0,0,800,600));

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


	if (IGUIFont* skinFont = guienv->getFont("bill/console.bmp"))
		skin->setFont(skinFont);

	canvas_.drawText(canvas_.loadFont("bill/console.bmp"), L"v0.1.1 (alpha)", anchored(rect<s32>(20,570,20,590), 0.0f, 1.0f, screen), SColor(255,255,255,255));

	guienv->drawAll();
	
}

void MainMenuScreen::hide()
{
	removeButtons(device_->getGUIEnvironment(), mainMenuButtons);
}

void PauseScreen::draw()// render the pause menu 
{
	IGUIEnvironment* guienv = device_->getGUIEnvironment();
	IGUISkin* skin = guienv->getSkin();
	dimension2d<u32> screen = canvas_.screenSize();
	canvas_.drawRect(anchored(rect<s32>(320,150,500,450), 0.5f, 0.5f, screen), SColor(150,0,0,0));

	position2d<s32> titleCorner = anchored(rect<s32>(290,150,530,188), 0.5f, 0.5f, screen).UpperLeftCorner;
	canvas_.drawImage(canvas_.loadTexture("bill/pauselogo.png"), rect<s32>(titleCorner, dimension2d<s32>(240,38)), rect<s32>(0,0,240,38));

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
	FontHandle deathFont = canvas_.loadFont("bill/bigfont.png");
	dimension2d<u32> screen = canvas_.screenSize();
	if (deathFont)
	{
		canvas_.drawText(deathFont, L"You died", rect<s32>(0, screen.Height/2 - 60, screen.Width, screen.Height/2), SColor(255,200,0,0), true, true);
		canvas_.drawText(deathFont, L"Press Esc for Main Menu", rect<s32>(0, screen.Height/2, screen.Width, screen.Height/2 + 40), SColor(255,255,255,255), true, true);
	}
}

void NoteScreen::draw(const ClueObject& clue)
{
	const string<char>& message = clue.getMessage();
	int width = 250;
	int height = 400;
	position2d<s32> corner = anchored(rect<s32>(500,100,500+width,100+height), 1.0f, 0.5f, canvas_.screenSize()).UpperLeftCorner;
	int posX = corner.X;
	int posY = corner.Y;

	int textHeigt = 50;
	int charWidth = 15;
	int textOffset = 40;

	FontHandle font = canvas_.loadFont("bill/console.bmp");

	canvas_.drawImage(canvas_.loadTexture("textures/bloodyNote.png"), rect<s32>(posX, posY, posX + width, posY + height), rect<s32>(0,0,width,height));

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
		
		canvas_.drawText(font, stringw(line).c_str(),
				   rect<s32>(posX+textOffset, posY+textOffset, 
							 posX+(line.size()*charWidth), posY+textOffset+textHeigt),
				   SColor(255,0,0,0));
	}
	//Draw the last line, since the quits before it prints. 
	//Might want to find a better way to do this
	line = message.subString(lineStart+1, message.size()-lineStart);
	posY += textOffset;
	canvas_.drawText(font, stringw(line).c_str(),
		rect<s32>(posX+textOffset, posY+textOffset, 
		posX+(line.size()*charWidth), posY+textOffset+textHeigt),
		SColor(255,0,0,0));
}

void drawIntro(IrrlichtDevice* device, Canvas& canvas)
{
	device->getVideoDriver()->beginScene(true, false, irr::video::SColor(255,0,0,0));

	canvas.drawImage(canvas.loadTexture("bill/intro.jpg"), letterboxed(canvas.screenSize()), rect<s32>(0,0,800,600));
	device->getGUIEnvironment()->drawAll();
	device->getVideoDriver()->endScene();


}

void drawLoading(IrrlichtDevice* device, Canvas& canvas)
{
	device->getVideoDriver()->beginScene(true, false, irr::video::SColor(255,0,0,0));

	canvas.loadTexture("bill/creepyman.jpg");
	canvas.loadTexture("bill/loading.png");

	device->getGUIEnvironment()->drawAll();

	canvas.drawText(canvas.loadFont("bill/bigfont.png"), L"LOADING...", anchored(rect<s32>(325,275,475,325), 0.5f, 0.5f, canvas.screenSize()), SColor(255,255,255,255));

	device->getVideoDriver()->endScene();


}
