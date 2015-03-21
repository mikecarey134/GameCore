#include "Event.h"
#include "Win_Launcher.h"


using namespace irr;
using namespace gui;

Event::~Event(void)
{
}

bool Event::OnEvent(const SEvent& event)
{
#ifdef __linux__ 
	//linux code goes here

#elif _WIN32

	Win_Launcher win_launch;
#endif

	if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();
		IGUIEnvironment* env = Context.device->getGUIEnvironment();

		switch(event.GUIEvent.EventType)
		{
		case EGET_SCROLL_BAR_CHANGED:

			if (id == GUI_ID_TRANSPARENCY_SCROLL_BAR)
			{
				//s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
				//setSkinTransparency(pos, env->getSkin());
			}
			break;
		case EGET_BUTTON_CLICKED:
			switch(id)
			{
			case GUI_ID_QUIT_BUTTON:
				printf("Quitting!\n");
				system("exit");
				Context.device->closeDevice();
				
			
				return true;
			case GUI_ID_START_BUTTON:

				#ifdef __linux__ 
				//linux code goes here

				#elif _WIN32
				win_launch.run_game();
				printf("Game Launching...\n");
				Context.device->closeDevice();
				
				return true;
				#endif
				break;
			case GUI_ID_SERVER_BUTTON:

				#ifdef __linux__ 
				//linux code goes here

				#elif _WIN32
				win_launch.run_server();
				printf("Server Launching...\n");
				Context.device->closeDevice();
				

				return true;
				#endif
				break;
			case GUI_ID_CONFIG_BUTTON:

				#ifdef __linux__ 
				//linux code goes here

				#elif _WIN32
				win_launch.open_config();
				printf("Opening Configuration File...\n");
				

				return true;
				#endif
				break;

			default:
				return false;
			}
			break;

		case EGET_FILE_SELECTED:
			{
			
			}
			break;

		default:
			break;
		}
	}

	return false;
}