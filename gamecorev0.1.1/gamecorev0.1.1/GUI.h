
/************************************************************************/
/* GUI.H
	Purpose: to draw the user interface 
	by: Michael Carey

*/
/************************************************************************/

#pragma once
#include<irrlicht.h>
#include "player.h"
#include "windows.h"
#include "psapi.h"
#include"gl/gl.h"

class GUI
{

public:
	enum events{GUI_ID_QUIT_BUTTON = 101,GUI_ID_NEW_WINDOW_BUTTON,
		GUI_ID_FILE_OPEN_BUTTON,
		GUI_ID_TRANSPARENCY_SCROLL_BAR};

	GUI(){}
	GUI(irr::IrrlichtDevice* device):device_(device), theColor_(255,255,255,255),red(255,255,0,0),blue(255,0,0,255),green(255,0,255,0),
		yellow(255,255,252,66),white(255,255,255,255){}//default colors
	
	GUI(const GUI& other): theColor_(other.theColor_), device_(other.device_)
	{
	}
	~GUI(void);

	void drawCrosshair       (irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location);
	void drawMessage         (irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location,irr::core::stringw msg);
	void drawHealth          (irr::gui::IGUIFont* font, player* thePlayer);
	void drawDebug           (irr::gui::IGUIFont* font,irr::video::IVideoDriver* driver,irr::core::vector3df nodePosition);
	void setSkinTransparency (irr::s32 alpha, irr::gui::IGUISkin * skin);
	void setColor            (irr::video::SColor newColor){theColor_ = newColor;}
	void drawloading(irr::video::IVideoDriver*& driver, irr::gui::IGUIEnvironment*& guienv);
	void drawIntro();
	void drawHUD(player* thePlayer); 
	void addDialogBox(wchar_t*);
	/*void drawchat(irr::gui::IGUIEnvironment*& guienv)
	{
	

		irr::gui::IGUIEditBox * ebox=guienv->addEditBox(irr::core::stringw("somevalue").c_str()
			,irr::core::recti(10,400, 200,580),true);
	
	}
*/

	//pre made colors
	//////////////////////////////////////////////////////////////////////////
	irr::video::SColor red;
	irr::video::SColor blue;
	irr::video::SColor yellow;
	irr::video::SColor green;
	irr::video::SColor white;	

private:	
	irr::video::SColor theColor_;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//irr::gui::IGUIEnvironment* guienv_;
	irr::IrrlichtDevice* device_;



};
