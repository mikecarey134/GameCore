#ifndef SCREENS_H
#define SCREENS_H

#include <irrlicht.h>

class player;

class Hud
{
public:
	explicit Hud(irr::IrrlichtDevice* device) : device_(device) {}
	void draw(const player& thePlayer);

private:
	irr::IrrlichtDevice* device_;
};

class MainMenuScreen
{
public:
	explicit MainMenuScreen(irr::IrrlichtDevice* device) : device_(device) {}
	void draw();
	void hide();

private:
	irr::IrrlichtDevice* device_;
};

class PauseScreen
{
public:
	explicit PauseScreen(irr::IrrlichtDevice* device) : device_(device) {}
	void draw();
	void hide();

private:
	irr::IrrlichtDevice* device_;
};

#endif
