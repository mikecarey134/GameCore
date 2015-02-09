
/************************************************************************/
/* GUI.H
	Purpose: to draw the user interface 
	by: Michael Carey

*/
/************************************************************************/

#pragma once
#include<irrlicht.h>

class GUI
{

public:
	enum events{GUI_ID_QUIT_BUTTON = 101,GUI_ID_NEW_WINDOW_BUTTON,
		GUI_ID_FILE_OPEN_BUTTON,
		GUI_ID_TRANSPARENCY_SCROLL_BAR};
	GUI():theColor_(255,255,255,255){}//default color white
	~GUI(void);

	void drawCrosshair       (irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location);
	void drawMessage         (irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location,irr::core::stringw msg);
	void drawHealth          (irr::gui::IGUIFont* font,irr::core::rect<irr::s32> location);
	void drawDebug           (irr::gui::IGUIFont* font,irr::video::IVideoDriver* driver,irr::core::vector3df nodePosition);
	void setSkinTransparency (irr::s32 alpha, irr::gui::IGUISkin * skin);
	void setColor            (irr::video::SColor newColor){theColor_ = newColor;}
	void drawloading(irr::video::IVideoDriver*& driver, irr::gui::IGUIEnvironment*& guienv);
	void drawchat(irr::gui::IGUIEnvironment*& guienv)
	{
	

		irr::gui::IGUIEditBox * ebox=guienv->addEditBox(irr::core::stringw("somevalue").c_str()
			,irr::core::recti(10,400, 200,580),true);
	
	}

private:
	irr::video::SColor theColor_;
	irr::gui::IGUIEnvironment* guienv_;
};
