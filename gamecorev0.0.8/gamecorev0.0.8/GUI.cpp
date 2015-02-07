/************************************************************************/
/* GUI.cpp
	Purpose: to draw the user interface 
	by: Michael Carey

*/
/************************************************************************/

#include "GUI.h"
#include "windows.h"
#include "psapi.h"
#include"gl/gl.h"


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;




GUI::~GUI(void)
{
}

void GUI::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
{
	for (s32 i=0; i<irr::gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
		col.setAlpha(alpha);
		skin->setColor((EGUI_DEFAULT_COLOR)i, col);
	}
}


void GUI::drawCrosshair(irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location)
{
		font->draw(L"^",//draw cross_hair
		location,theColor_);//set to private member color

}
void GUI::drawHealth(IGUIFont* font,rect<s32> location)
{
	font->draw(L"Health: 100",//draw health icon
	location,theColor_);

}
void GUI::drawDebug(IGUIFont* font,IVideoDriver* driver,vector3df nodePosition)
{

	int GUIFps = driver->getFPS();
	int pDrawn = driver->getPrimitiveCountDrawn();


	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;

	//PROCESS_MEMORY_COUNTERS_EX pmc;
	//GetProcessMemoryInfo(GetCurrentProcess(),&pmc, sizeof(pmc));
	//size_t virtualMemUsedByMe = pmc.PrivateUsage;

	core::stringw fpsstr;
	fpsstr+="---GameCorev0.0.8---\n";
	fpsstr+= " FPS:";
	fpsstr+=GUIFps;
	fpsstr+= "\n VMemory KB:";
	fpsstr+= (long)virtualMemUsed / 2048;
	fpsstr+= "\n Prim Drawn:";
	fpsstr+=pDrawn;
	fpsstr+= "\n Tex Loaded:";
	fpsstr+=driver->getTextureCount();
	fpsstr+="\n X:";
	fpsstr+= nodePosition.X;
	fpsstr+="\n Y:";
	fpsstr+= nodePosition.Y;
	fpsstr+="\n Z:";
	fpsstr+= nodePosition.Z;
	fpsstr+="\n Vendor:";
	fpsstr+=driver->getVendorInfo();//get card vendor info
	fpsstr+="\n Card:";
	//fpsstr+=driver->getVendorInfo();
	fpsstr+=glGetString(GL_RENDERER);//get graphics card name

	fpsstr+="\n Driver:";
	fpsstr+= driver->getName();
	fpsstr+="\n Debug:";
	fpsstr+= driver->getDebugName();
	
	
	
	fpsstr+="\n-------------------------";

	driver->draw2DRectangle(SColor(100,0,0,0), rect<s32>(670,10,790,180));

	font->draw(fpsstr,//draw current fps for debugging
		rect<s32>(680,20,680,0),theColor_);//set up the color

}
//draw a message in a specified location
void GUI::drawMessage(irr::gui::IGUIFont* font ,irr::core::rect<irr::s32> location,stringw msg)
{
	font->draw(msg,//draw current fps for debugging
		location,theColor_);
}

//temporary function in order to draw a loading screen whilst loading data
void GUI::drawloading(irr::video::IVideoDriver*& driver, irr::gui::IGUIEnvironment*& guienv)
{
	driver->beginScene(true, false, irr::video::SColor(0,0,0,0));

	ITexture* backgroundImage = driver->getTexture("bill/creepyman.jpg");
	ITexture* titleImage = driver->getTexture("bill/loading.jpg");

	//driver->draw2DImage(backgroundImage,position2d<s32>(0,0),rect<s32>(0,0,800,600));
	//driver->draw2DImage(titleImage,position2d<s32>(100,100),rect<s32>(40,40,600,300),NULL,SColor(255,255,255,255), false);

	guienv->drawAll();



	irr::gui::IGUIFont* font = guienv->getFont("bill/bigfont.png");

	font->draw(L"LOADING...", irr::core::rect<irr::s32>(325,275,475,325), irr::video::SColor(255,255,255,255));

	driver->endScene();


}