#ifndef GAMELAYERS_H
#define GAMELAYERS_H

#include "InputRouter.h"

class consoleevent;

class ConsoleLayer : public InputLayer
{
public:
	explicit ConsoleLayer(consoleevent& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
};

#endif
