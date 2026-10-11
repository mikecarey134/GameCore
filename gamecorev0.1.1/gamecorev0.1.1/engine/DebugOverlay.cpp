#include "DebugOverlay.h"
#include "UiAnchor.h"
#include <irrbullet.h>
#ifdef _WIN32
#include "windows.h"
#include "psapi.h"
#include "gl/gl.h"
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#ifdef __linux__
#include <sys/sysinfo.h>
#endif

using namespace irr;
using namespace core;
using namespace video;
using namespace gui;

void DebugOverlay::draw(const vector3df& nodePosition)
{
	IVideoDriver* driver = device_->getVideoDriver();
	IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();

	//add all the info to the screen
	int GUIFps = driver->getFPS();
	int pDrawn = driver->getPrimitiveCountDrawn();


#ifdef _WIN32
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
#elif defined(__linux__)
	struct sysinfo memInfo;
	sysinfo(&memInfo);
	unsigned long long virtualMemUsed =
		((unsigned long long)memInfo.totalram + memInfo.totalswap
		 - memInfo.freeram - memInfo.freeswap) * memInfo.mem_unit;
#else
	unsigned long long virtualMemUsed = 0;
#endif

	//PROCESS_MEMORY_COUNTERS_EX pmc;
	//GetProcessMemoryInfo(GetCurrentProcess(),&pmc, sizeof(pmc));
	//size_t virtualMemUsedByMe = pmc.PrivateUsage;

	core::stringw fpsstr;
	fpsstr+="---GameCorev0.1.1---\n";
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
	if (driver->getDriverType() == EDT_OPENGL)
		fpsstr+=glGetString(GL_RENDERER);//get graphics card name

	fpsstr+="\n Driver:";
	fpsstr+= driver->getName();
	fpsstr+="\n Debug:";
	fpsstr+= driver->getDebugName();
	
	
	
	fpsstr+="\n-------------------------";

	driver->draw2DRectangle(SColor(100,0,0,0), anchored(rect<s32>(670,10,790,180), 1.0f, 0.0f, driver->getScreenSize()));

	font->draw(fpsstr,//draw current fps for debugging
		anchored(rect<s32>(680,20,680,0), 1.0f, 0.0f, driver->getScreenSize()),SColor(255,255,255,255));//set up the color

	world_->debugDrawWorld(true);
}
