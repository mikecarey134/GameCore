#ifndef CONSOLELAYER_H
#define CONSOLELAYER_H

#include <functional>
#include "InputRouter.h"
#include "console.h"
#include "ActionMap.h"

class ConsoleLayer : public InputLayer
{
public:
	ConsoleLayer(IC_Console& console, const ActionMap& actions, std::function<bool()> canToggle = nullptr)
		: console_(console), actions_(actions), canToggle_(canToggle) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	IC_Console& console_;
	const ActionMap& actions_;
	std::function<bool()> canToggle_;
};

#endif
