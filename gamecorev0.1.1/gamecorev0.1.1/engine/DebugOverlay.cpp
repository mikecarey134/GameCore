#include "DebugOverlay.h"
#include <irrlicht.h>
#include <string>
#include "UiAnchor.h"
#include "physics/PhysicsWorld.h"
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
#include <unistd.h>
#include <cstdio>
#endif

using namespace irr;
using namespace core;
using namespace video;

namespace
{
	void appendWrapped(core::stringw& out, const char* text, std::size_t width)
	{
		if (!text)
			return;
		const std::string s(text);
		for (std::size_t i = 0; i < s.size(); i += width)
		{
			out += "\n  ";
			out += core::stringw(s.substr(i, width).c_str());
		}
	}
}

void DebugOverlay::draw(const vector3df& nodePosition, const irr::core::stringw& playerDirection, const irr::core::stringw& cameraDirection)
{
	IVideoDriver* driver = device_->getVideoDriver();

	//add all the info to the screen
	int GUIFps = driver->getFPS();
	int pDrawn = driver->getPrimitiveCountDrawn();

	
#ifdef _WIN32
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	const char* memoryLabel = "\nSystem Page File MB:";
#elif defined(__linux__)
	// Resident memory of this process (statm fields are in pages)
	unsigned long long virtualMemUsed = 0;
	if (FILE* f = fopen("/proc/self/statm", "r"))
	{
		unsigned long long sizePages = 0, residentPages = 0;
		if (fscanf(f, "%llu %llu", &sizePages, &residentPages) == 2)
			virtualMemUsed = residentPages * (unsigned long long)sysconf(_SC_PAGESIZE);
		fclose(f);
	}
	const char* memoryLabel = "\nProcess RSS MB:";
#else
	unsigned long long virtualMemUsed = 0;
	const char* memoryLabel = "\nMemory MB:";
#endif

	//PROCESS_MEMORY_COUNTERS_EX pmc;
	//GetProcessMemoryInfo(GetCurrentProcess(),&pmc, sizeof(pmc));
	//size_t virtualMemUsedByMe = pmc.PrivateUsage;
	const int offset = 100;
	core::stringw fpsstr;
	fpsstr+="---GameCorev0.1.1---\n";
	fpsstr+= "FPS:";
	fpsstr+=GUIFps;
	fpsstr+= memoryLabel;
	fpsstr+= static_cast<long>(virtualMemUsed / (1024ULL * 1024ULL)); // Convert bytes to MB
	fpsstr+= "\nPrim Drawn:";
	fpsstr+=pDrawn;
	fpsstr+= "\nTex Loaded:";
	fpsstr+=driver->getTextureCount();
	fpsstr+="\nPlayer Facing:";
	fpsstr+= playerDirection;
	fpsstr+="\nX:";
	fpsstr+= nodePosition.X;
	fpsstr+="\nY:";
	fpsstr+= nodePosition.Y;
	fpsstr+="\nZ:";
	fpsstr+= nodePosition.Z;
	fpsstr+="\nVendor:";
	fpsstr+=driver->getVendorInfo();//get card vendor info
	fpsstr+="\nCard:";
	if (driver->getDriverType() == EDT_OPENGL)
		appendWrapped(fpsstr, reinterpret_cast<const char*>(glGetString(GL_RENDERER)), 15);
	fpsstr+="\nDriver:";
	fpsstr+= driver->getName();
	fpsstr+="\nDebug:";
	fpsstr+= driver->getDebugName();
	fpsstr+="\nCamera Facing:";
	fpsstr+= cameraDirection;
	
	fpsstr+="\n-------------------------";
	// Calculate the dynamic position for the top-right corner
	dimension2d<u32> screenSizeDim = canvas_.screenSize();
	rect<s32> screenSize(0, 0, static_cast<s32>(screenSizeDim.Width), static_cast<s32>(screenSizeDim.Height));
	s32 width = 200;
	s32 height = 280;
	s32 x = screenSize.LowerRightCorner.X - width - 10; // 10px padding from the right
	s32 y = 10; // 10px padding from the top

	// Draw the background rectangle
	canvas_.drawRect(anchored(rect<s32>(x, y, x + width, y + (height+offset)), 0.0f, 0.0f, screenSizeDim), SColor(100, 0, 0, 0));

	canvas_.drawText(font_ ? font_ : canvas_.builtInFont(), fpsstr.c_str(),
		anchored(rect<s32>(x + 10, y + 10, x + width - 10, y + height - 10), 0.0f, 0.0f, screenSizeDim), SColor(255, 255, 255, 255));

	if (physics_)
		physics_->drawDebug(driver);
}
