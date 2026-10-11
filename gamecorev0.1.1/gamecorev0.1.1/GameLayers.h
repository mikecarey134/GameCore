#ifndef GAMELAYERS_H
#define GAMELAYERS_H

#include "InputRouter.h"

class consoleevent;
namespace gamecore { class SoundEngine; }

class ConsoleLayer : public InputLayer
{
public:
	explicit ConsoleLayer(consoleevent& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
};

class NoteLayer : public InputLayer
{
public:
	explicit NoteLayer(consoleevent& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
};

class InventoryLayer : public InputLayer
{
public:
	InventoryLayer(consoleevent& game, gamecore::SoundEngine* sound) : game_(game), sound_(sound) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
	gamecore::SoundEngine* sound_;
};

class MenuLayer : public InputLayer
{
public:
	explicit MenuLayer(consoleevent& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
};

class DebugLayer : public InputLayer
{
public:
	explicit DebugLayer(consoleevent& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	consoleevent& game_;
};

#endif
