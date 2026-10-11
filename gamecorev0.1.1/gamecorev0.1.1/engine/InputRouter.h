#ifndef INPUTROUTER_H
#define INPUTROUTER_H

#include <IEventReceiver.h>
#include <vector>
#include "InputState.h"

class InputLayer
{
public:
	virtual ~InputLayer() {}
	virtual bool onEvent(const irr::SEvent& event) = 0;
	virtual bool isActive() const { return true; }
};

class InputRouter : public irr::IEventReceiver
{
public:
	void push(InputLayer* layer);
	void remove(InputLayer* layer);
	bool OnEvent(const irr::SEvent& event) override;
	const InputState& state() const { return state_; }

private:
	std::vector<InputLayer*> layers_;
	InputState state_;
};

#endif
