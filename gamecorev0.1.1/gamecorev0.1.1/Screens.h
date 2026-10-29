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
	MainMenuScreen(irr::IrrlichtDevice* device, Canvas& canvas) : device_(device), canvas_(canvas) {}
	void draw();
	void hide();

private:
	irr::IrrlichtDevice* device_;
	Canvas& canvas_;
};

class PauseScreen
{
public:
	PauseScreen(irr::IrrlichtDevice* device, Canvas& canvas) : device_(device), canvas_(canvas) {}
	void draw();
	void hide();

private:
	irr::IrrlichtDevice* device_;
	Canvas& canvas_;
};

class DeathScreen
{
public:
	explicit DeathScreen(Canvas& canvas) : canvas_(canvas) {}
	void draw();

private:
	Canvas& canvas_;
};

class NoteScreen
{
public:
	explicit NoteScreen(Canvas& canvas) : canvas_(canvas) {}
	void draw(const ClueObject& clue);

private:
	Canvas& canvas_;
};

void drawIntro(irr::IrrlichtDevice* device, Canvas& canvas);
void drawLoading(irr::IrrlichtDevice* device, Canvas& canvas);

#endif
