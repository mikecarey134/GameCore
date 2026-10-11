#ifndef GAMELAYERS_H
#define GAMELAYERS_H

#include "InputRouter.h"

class GameFlow;
namespace gamecore { class SoundEngine; }

class ConsoleLayer : public InputLayer
{
public:
	explicit ConsoleLayer(GameFlow& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
};

class NoteLayer : public InputLayer
{
public:
	explicit NoteLayer(GameFlow& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
};

class InventoryLayer : public InputLayer
{
public:
	InventoryLayer(GameFlow& game, gamecore::SoundEngine* sound) : game_(game), sound_(sound) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	gamecore::SoundEngine* sound_;
};

class MenuLayer : public InputLayer
{
public:
	explicit MenuLayer(GameFlow& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
};

class DebugLayer : public InputLayer
{
public:
	explicit DebugLayer(GameFlow& game) : game_(game) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
};

class GameplayLayer : public InputLayer
{
public:
	GameplayLayer(GameFlow& game, const InputState& input) : game_(game), input_(input) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	const InputState& input_;
};

#endif
