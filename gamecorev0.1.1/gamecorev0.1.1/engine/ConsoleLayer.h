#ifndef CONSOLELAYER_H
#define CONSOLELAYER_H

#include <functional>
#include "InputRouter.h"
#include "console.h"

class ConsoleLayer : public InputLayer
{
public:
	explicit ConsoleLayer(IC_Console& console, std::function<bool()> canToggle = nullptr)
		: console_(console), canToggle_(canToggle) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	IC_Console& console_;
	std::function<bool()> canToggle_;
};

#endif
