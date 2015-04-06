

#ifdef __linux__ 
//linux code goes here
#include<unistd.h>
#include<sys/wait.h>


#elif _WIN32
//windows code goes here
#include<Windows.h>
#include <fstream>
#include"Win_Launcher.h"
#pragma comment(lib,"Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")//comment out if you want to see the Debug console
#else
#endif

#include<irrlicht.h>
#include<stdlib.h>
#include "Event.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace gui;


int main(int argc, char **argv)
{


	const int windowWidth = 800;
	const int windowHeight = 500;

	IrrlichtDevice* device = createDevice(E_DRIVER_TYPE::EDT_OPENGL,core::dimension2d<u32>(windowWidth,windowHeight));
	if(!device)
		exit(-1);

	IVideoDriver* driver = device->getVideoDriver();
	irr::scene::ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* env = device->getGUIEnvironment();

	IGUISkin* skin = env->getSkin();
	
	ITexture* logo =driver->getTexture("banner.png");
	//driver->draw2DImage(logo,rect<s32>(10,10,190,100),rect<s32>(10,10,190,100));

	IGUIFont* font = env->getFont("fonts/newsfont.xml");
	skin->setColor(EGDC_BUTTON_TEXT, SColor(255,255,255,255));
	if (font)
		skin->setFont(font);
	

	int YPos = 440;
	int YHeight = YPos + 45;

	int XPos = 10;
	int XWidth = 150;
	int XOffset = 55;

	env->addButton(rect<s32>(XPos,YPos,XPos+XWidth,YHeight), 0,Event::GUI_ID_START_BUTTON,
		L"Game Launch", L"Launches the Game");
	XPos += XWidth + XOffset;
	env->addButton(rect<s32>(XPos,YPos,XPos+XWidth,YHeight), 0, Event::GUI_ID_SERVER_BUTTON,
		L"Run Server", L"Starts the Server");
	XPos += XWidth + XOffset;
	env->addButton(rect<s32>(XPos,YPos,XPos+XWidth,YHeight), 0, Event::GUI_ID_CONFIG_BUTTON,
		L"Configure", L"Edit Configuration File");
	XPos += XWidth + XOffset;
	env->addButton(rect<s32>(XPos,YPos,XPos+XWidth,YHeight), 0, Event::GUI_ID_QUIT_BUTTON,
		L"Quit", L"Exits Program");
	// Store the appropriate data in a context structure.
	SAppContext context;
	context.device = device;
	context.counter = 0;
	
	IGUIListBox* news_list_box = env->addListBox(core::rect<s32>(0,70, windowWidth, 390));
	context.listbox = news_list_box;
	

	std::ifstream news_file;
	std::string news;
	news_file.open("newsfile.txt");
	if (news_file.fail())
		context.listbox->addItem(L"News read fail");
	else
	{
		while (std::getline(news_file,news))
		{ 
			wchar_t* wnews = new wchar_t[news.size()];
			mbstowcs(wnews,news.c_str(),news.size());

			context.listbox->addItem(wnews);

			delete wnews;
		}
	}

	// Then create the event receiver, giving it that context structure.
	Event receiver(context);

	// And tell the device to use our custom event receiver.
	device->setEventReceiver(&receiver);


	device->setWindowCaption(L"Game Launcher v0.2");
	while(device->run())
	{
		driver->beginScene(true,true, SColor(255,0,0,0));

		driver->draw2DRectangle(core::rect<s32>(0,0,windowWidth,70),
			irr::video::SColor(255, 0,0,0), irr::video::SColor(255, 0,0,0),
			irr::video::SColor(255, 150,0,0), irr::video::SColor(255, 150,0,0));

		driver->draw2DRectangle(core::rect<s32>(0,390,windowWidth,windowHeight),
			irr::video::SColor(255, 150,0,0), irr::video::SColor(255, 150,0,0),
			irr::video::SColor(255, 0,0,0), irr::video::SColor(255, 0,0,0));

		env->drawAll();
		smgr->drawAll();
		driver->draw2DImage(logo,position2d<s32>(270,16));
		driver->endScene();

	}	
	device->drop();
	return 0;
}





