/************************************************************************/
/*
    splash.cpp
	Purpose: create a splash screen 
	By:Michael Carey

*/
/************************************************************************/



#pragma once

#include "Splash.h"
#include "GUI.h"


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


void Splash::createSplash()
{
	GUI theGUI;
	IrrlichtDevice *device =
		createDevice( video::EDT_OPENGL, dimension2d<u32>(640, 480), 16,
		false, false, false, 0);

	if (!device){
		exit(1);
	}

	IVideoDriver* driver = device->getVideoDriver();

	irr::video::ITexture* image = driver->getTexture("bill/load.png");
	driver->makeColorKeyTexture(image,position2d<s32>(0,0));
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	IGUIFont* font =guienv->getFont("bill/bigfont.png");
	int i = 100000;



	while(device->run())
	{

		u32 time = device->getTimer()->getTime();
		int currentframe = 0,row = 0;

		if((time % 75) == 0) currentframe ++;

		if (currentframe >= 13)
		{
			currentframe = 0;
			row ++;
		}
		if(row >= 12)row = 0;


		driver->beginScene(true, true, SColor(255,255,255,255));

		driver->draw2DImage(image,position2d<s32>(200,200),rect<s32>(currentframe*64,row*64,(currentframe+1)*64,(row+1)*64),
			0,SColor(255,255,255,255),true);

		font->draw(L"LOADING GAME...",rect<s32>(300,300,300,50),SColor(255,255,255,255));

		driver->endScene();
		i--;
	}

	device->drop();


	
}


