

#ifdef __linux__ 
//linux code goes here
#include<unistd.h>
#include<sys/wait.h>


#elif _WIN32
//windows code goes here
#include<Windows.h>
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




	IrrlichtDevice* device = createDevice(E_DRIVER_TYPE::EDT_OPENGL,core::dimension2d<u32>(200,400));
	if(!device)
		exit(-1);

	IVideoDriver* driver = device->getVideoDriver();
	irr::scene::ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* env = device->getGUIEnvironment();

	IGUISkin* skin = env->getSkin();
	
	ITexture* logo =driver->getTexture("hc.jpg");
	driver->draw2DImage(logo,rect<s32>(10,10,190,100),rect<s32>(10,10,190,100));

	skin->setFont(env->getBuiltInFont(), EGDF_TOOLTIP);

	env->addButton(rect<s32>(10,240,110,240 + 32), 0,Event::GUI_ID_START_BUTTON,
		L"Game Launch", L"Launches the Game");
	env->addButton(rect<s32>(10,280,110,280 + 32), 0, Event::GUI_ID_SERVER_BUTTON,
		L"Run Server", L"Starts the Server");
	env->addButton(rect<s32>(10,320,110,320 + 32), 0, Event::GUI_ID_CONFIG_BUTTON,
		L"Configuration", L"Edit Configuration File");
	env->addButton(rect<s32>(10,360,110,360 + 32), 0, Event::GUI_ID_QUIT_BUTTON,
		L"Quit", L"Exits Program");

	// Store the appropriate data in a context structure.
	SAppContext context;
	context.device = device;
	context.counter = 0;

	// Then create the event receiver, giving it that context structure.
	Event receiver(context);

	// And tell the device to use our custom event receiver.
	device->setEventReceiver(&receiver);


	device->setWindowCaption(L"Game Launcher v0.1");
	while(device->run())
	{
		driver->beginScene();
		env->drawAll();
		smgr->drawAll();
		driver->draw2DImage(logo,rect<s32>(10,10,190,200),rect<s32>(10,10,190,100));
		driver->endScene();

	}	
	device->drop();
	return 0;
}





