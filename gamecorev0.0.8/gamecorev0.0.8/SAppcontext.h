/************************************************************************/
/*
    SAppcontext.h
	Purpose: handles GUI information for GUI based events 
	By:Michael Carey

*/
/************************************************************************/

#include<irrlicht.h>
#pragma once

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


struct SAppContext
{
	IrrlichtDevice *device;
	s32				counter;
	IGUIListBox*	listbox;
};