#pragma once
#include <irrlicht.h>


struct SAppContext
{
	irr::IrrlichtDevice      *device;
	irr::s32                  counter;
	irr::gui::IGUIListBox*    listbox;
};

class Event :public irr::IEventReceiver
{
public:
	enum
	{
		GUI_ID_QUIT_BUTTON = 101,
		GUI_ID_START_BUTTON,
		GUI_ID_SERVER_BUTTON,
		GUI_ID_CONFIG_BUTTON,
		GUI_ID_TRANSPARENCY_SCROLL_BAR
	};

	Event(SAppContext context):Context(context){}
	~Event(void);
	virtual bool OnEvent(const irr::SEvent& event);
private:
	SAppContext Context;
};
