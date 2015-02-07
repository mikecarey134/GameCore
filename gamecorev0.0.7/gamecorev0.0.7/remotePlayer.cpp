#include "remotePlayer.h"

/************************************************************************/
/* Player.cpp
	purpose: to render the character and animate them
	By: Michael Carey
*/
/************************************************************************/


#include "player.h"
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>

#include<string>
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"
#include "playerData.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


remotePlayer::remotePlayer(IrrlichtDevice* device,char* filename,irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, ISoundEngine* engine, 
irrBulletWorld* world):
device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world),
xDirection_(0.0f), zDirection_(0.0f),playerHealth_(PLAYER_HEALTH)
{
	
	float height = 5.0;
	float width  = 3.0;

	character_ = new IKinematicCharacterController(world_);
	character_->warp(vector3df(41.0f, 73.0f, 23.0f));//set the init player pos on the map

	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh(filename));
	
	setTexture("characters/playerskin3.jpg");
	characterModel_->setScale(vector3df(0.75, 0.75, 0.75));
	
	characterModel_->setAnimationSpeed(PLAYER_ANIMATION_SPEED);
	
	
	character_->setGravity(PLAYER_EARTH_GRAVITY);

	character_->setJumpForce(PLAYER_JUMP_FORCE);

	character_->setMaxSlope(PI/4);

	playerSteps_ = engine_->play2D("sounds/footsteps-4.wav", true, true); //Player foot steps sounds. Declare them here and start it off as paused
	
	playerSpeed_ = DEFAULT_PLAYER_SPEED;

	//////////////////////////////////////////////////////////////////////////
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,1);
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,0);
	//characterModel_->setID(ID_IsNotPickable);//so we cannot select our model
	characterModel_->addShadowVolumeSceneNode();//make realtime shadows on the character
	//////////////////////////////////////////////////////////////////////////
	
	//idle();

	//lamp_= device_->getSceneManager()->addLightSceneNode(0,characterModel_->getPosition(),
		//SColor(150,237,245,157),12);//add our lamp here
}

remotePlayer::~remotePlayer(void)
{
	
}
void remotePlayer::animate(EMD2_ANIMATION_TYPE animation)
{

	
	characterModel_->setMaterialFlag(video::EMF_LIGHTING, true);
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	
	characterModel_->setMD2Animation(animation);
}


/*
void remotePlayer::jump()
{
	if (character_->isOnGround()){
		character_->jump();
		characterModel_->setFrameLoop(103,111);
	}
}


void remotePlayer::forward()
{	
	characterModel_->setFrameLoop(301,318);
	characterModel_->setAnimationSpeed(4);

	/*if (walkframe_== 0 || walkframe_ == 319 )
	{
		walkframe_ = 301;
	}
	else
	{
		walkframe_++;
	}

	characterModel_->setCurrentFrame(walkframe_);
}
void remotePlayer::backwards()
{	
	
}
void remotePlayer::left()
{
}
void remotePlayer::right()
{	
}




void remotePlayer::idle()
{
	characterModel_->setFrameLoop(206,250);
}
*/
