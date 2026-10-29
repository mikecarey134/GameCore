#ifndef INPUTSTATE_H
#define INPUTSTATE_H

#include <IEventReceiver.h>

class InputState
{
public:
	InputState()
	{
		for (int i = 0; i < irr::KEY_KEY_CODES_COUNT; ++i)
			keys_[i] = false;
	}

	void update(const irr::SEvent& event)
	{
		if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.Key < irr::KEY_KEY_CODES_COUNT)
			keys_[event.KeyInput.Key] = event.KeyInput.PressedDown;
		else if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
		{
			switch (event.MouseInput.Event)
			{
			case irr::EMIE_LMOUSE_PRESSED_DOWN: keys_[irr::KEY_LBUTTON] = true; break;
			case irr::EMIE_LMOUSE_LEFT_UP: keys_[irr::KEY_LBUTTON] = false; break;
			case irr::EMIE_RMOUSE_PRESSED_DOWN: keys_[irr::KEY_RBUTTON] = true; break;
			case irr::EMIE_RMOUSE_LEFT_UP: keys_[irr::KEY_RBUTTON] = false; break;
			case irr::EMIE_MMOUSE_PRESSED_DOWN: keys_[irr::KEY_MBUTTON] = true; break;
			case irr::EMIE_MMOUSE_LEFT_UP: keys_[irr::KEY_MBUTTON] = false; break;
			default: break;
			}
		}
	}

	bool isKeyDown(irr::EKEY_CODE key) const { return keys_[key]; }
	bool isKeyUp(irr::EKEY_CODE key) const { return !keys_[key]; }

private:
	bool keys_[irr::KEY_KEY_CODES_COUNT];
};

#endif
