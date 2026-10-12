#ifndef SCREENS_H
#define SCREENS_H

#include <irrlicht.h>
#include "Canvas.h"

class player;
class ClueObject;

class Hud
{
public:
	explicit Hud(Canvas& canvas) : canvas_(canvas) {}
	void draw(const player& thePlayer);

private:
	Canvas& canvas_;
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

class DeathScreen
{
public:
	explicit DeathScreen(irr::IrrlichtDevice* device) : device_(device) {}
	void draw();

private:
	irr::IrrlichtDevice* device_;
};

class NoteScreen
{
public:
	explicit NoteScreen(irr::IrrlichtDevice* device) : device_(device) {}
	void draw(const ClueObject& clue);

private:
	irr::IrrlichtDevice* device_;
};

void drawIntro(irr::IrrlichtDevice* device);
void drawLoading(irr::IrrlichtDevice* device);

#endif
