#ifndef GAMELAYERS_H
#define GAMELAYERS_H

#include "InputRouter.h"
#include "ActionMap.h"

class GameFlow;
namespace gamecore { class SoundEngine; }

void bindDefaultActions(ActionMap& actions);

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
	InventoryLayer(GameFlow& game, const ActionMap& actions, gamecore::SoundEngine* sound) : game_(game), actions_(actions), sound_(sound) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	const ActionMap& actions_;
	gamecore::SoundEngine* sound_;
};

class MenuLayer : public InputLayer
{
public:
	MenuLayer(GameFlow& game, const ActionMap& actions) : game_(game), actions_(actions) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	const ActionMap& actions_;
};

class DebugLayer : public InputLayer
{
public:
	DebugLayer(GameFlow& game, const ActionMap& actions) : game_(game), actions_(actions) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	const ActionMap& actions_;
};

class GameplayLayer : public InputLayer
{
public:
	GameplayLayer(GameFlow& game, const ActionMap& actions, const InputState& input) : game_(game), actions_(actions), input_(input) {}
	bool onEvent(const irr::SEvent& event) override;

private:
	GameFlow& game_;
	const ActionMap& actions_;
	const InputState& input_;
};

#endif
