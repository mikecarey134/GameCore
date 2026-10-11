#include "Screens.h"
#include "player.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace gui;

namespace
{
	const SColor red(255,255,0,0);
	const SColor blue(255,0,0,255);
	const SColor yellow(255,255,252,66);
	const SColor green(255,0,255,0);
	const SColor white(255,255,255,255);
}

void Hud::draw(const player& thePlayer)
{
	IVideoDriver* driver = device_->getVideoDriver();

	SColor healthColor;
	//draw health color based on how much health they have
	if(thePlayer.getHealth() >=60 && thePlayer.getHealth() < 101){healthColor = green;}
	else if(thePlayer.getHealth() >=30 && thePlayer.getHealth() < 60){healthColor = yellow;}
	else if(thePlayer.getHealth() >=1 && thePlayer.getHealth() < 30){healthColor = red;}
	else healthColor = blue;

	driver->draw2DRectangle(SColor(150,0,0,0), rect<s32>(290,530, 560,570));
	driver->draw2DRectangle(healthColor, rect<s32>(300,540, 300+core::clamp(thePlayer.getHealth(),0,100)*5/2,560));

	//draw the health bar
	driver->draw2DRectangle(SColor(150,0,0,0), rect<s32>(660,458, 760,570));

	//draw the current weapon
	ITexture* currentWeapon = driver->getTexture("bill/icons/hand-small.jpg");
	driver->draw2DImage(currentWeapon, position2d<s32>(670,470),rect<s32>(0,0,80,90));

	IGUIFont* font2 = device_->getGUIEnvironment()->getFont("bill/console.bmp");
	core::stringw fpsstr;//draw the current hunger and water levels
	fpsstr += "Water:";
	
	font2->draw(fpsstr,//draw current fps for debugging
		rect<s32>(370,480, 470 ,560),white);//set up the color

	SColor color;
	//draw health color based on how much health they have
	if(thePlayer.getWater() >=60 && thePlayer.getWater() < 101){color = blue;}
	else if(thePlayer.getWater() >=30 && thePlayer.getWater() < 60){color = yellow;}
	else if(thePlayer.getWater() >=1 && thePlayer.getWater() < 30){color = red;}
	fpsstr="";
	fpsstr += thePlayer.getWater();
	font2->draw(fpsstr,//draw current fps for debugging
		rect<s32>(410,480, 510 ,560),color);//set up the color
}
