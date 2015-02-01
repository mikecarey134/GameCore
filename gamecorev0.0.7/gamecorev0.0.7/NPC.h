/************************************************************************/
/* NPC.h
	purpose: To make a functional npc for the game
	By: Dan Brown
*/
/************************************************************************/

#pragma once
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>
#include<irrbullet.h>

#include<lua.hpp>

#include<string>
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"

#define NPC_EARTH_GRAVITY 150.0f

class NPC
{
private:
	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	irrBulletWorld* world_;

	IKinematicCharacterController* character_;
	irr::scene::IAnimatedMeshSceneNode* characterModel_;

	irr::core::vector3df AIdirection_;
	irr::u32 directionCounter_;
	
public:
	NPC(irr::IrrlichtDevice* device, /*char* filename,*/ irr::scene::ISceneManager* smgr,
		irrBulletWorld* world, irr::video::IVideoDriver* driver);

	~NPC(void);

	void setposition   (irr::core::vector3df pos) { character_->warp(pos); }
	void moveNPC ();
	void luaSetDir();
};