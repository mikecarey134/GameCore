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
	}

	bool isKeyDown(irr::EKEY_CODE key) const { return keys_[key]; }
	bool isKeyUp(irr::EKEY_CODE key) const { return !keys_[key]; }

private:
	bool keys_[irr::KEY_KEY_CODES_COUNT];
};

#endif
