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

#include<lua.hpp>

#include<string>
#include"common.h"
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"
#include"player.h"
#include"GUI.h"

#define NPC_EARTH_GRAVITY 150.0f
#define NPC_HEALTH 100
#define NPC_SPEED 0.25

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
	int npcHealth_;
	loadState isLoaded_;
	bool isHit_;
	bool isDead_;

	irr::scene::IBillboardTextSceneNode* healthDisplay_;
	irr::core::stringw healthText_;
	
public:
	NPC(irr::IrrlichtDevice* device, /*char* filename,*/ irr::scene::ISceneManager* smgr,
		irrBulletWorld* world, irr::video::IVideoDriver* driver);

	~NPC(void);

	void setposition   (irr::core::vector3df pos) { character_->warp(pos); }
	void moveNPC   ();
	void luaSetDir ();
	int  getHealth ()				{ return npcHealth_; }
	void setHealth (int newHealth)	{ npcHealth_ = newHealth; }
	void damage();
	loadState& isLoaded()			{ return isLoaded_; }
	bool& isHit()					{ return isHit_; }
	bool& isDead()					{ return isDead_; }
	irr::core::vector3df getPosition() { return characterModel_->getPosition(); }

	void loadModel();

	void drawNPCHealth();

	IKinematicCharacterController* getKinematicChar() { return character_; }
};