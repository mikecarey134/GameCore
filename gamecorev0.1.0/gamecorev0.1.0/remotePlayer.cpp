

/************************************************************************/
/* remotePlayer.cpp
	purpose: to render client characters and animate them
	By: Michael Carey
*/
/************************************************************************/


#include "remotePlayer.h"
#include "player.h"
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>
#include<string>
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"


using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


remotePlayer::remotePlayer(IrrlichtDevice* device,irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver,
						   ISoundEngine* engine, irrBulletWorld* world,int model_type):
device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world),
xDirection_(0.0f), zDirection_(0.0f),playerHealth_(PLAYER_HEALTH)
{
	
	float height = 5.0;
	float width  = 3.0;

	character_ = new IKinematicCharacterController(world_);
	character_->warp(vector3df(113.0f, 38.0f, 25.0f));//set the init player pos on the map

	//characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh(filename));
	
	//setTexture("characters/playerskin_m2.jpg");
	characterModel_ = smgr_->addAnimatedMeshSceneNode(smgr_->getMesh(paths_.model_paths[model_type].c_str()));

	//setTexture("characters/playerskin3.jpg");
	setTexture(paths_.skin_paths[model_type].c_str());
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));
	
	characterModel_->setAnimationSpeed(PLAYER_ANIMATION_SPEED);
	characterModel_->setPosition(vector3df(113.0f, 38.0f, 25.0f));
	
	character_->setGravity(PLAYER_EARTH_GRAVITY);

	character_->setJumpForce(PLAYER_JUMP_FORCE);

	character_->setMaxSlope(PI/4);

	playerSteps_ = engine_->play2D("sounds/footsteps-4.wav", true, true); //Player foot steps sounds. Declare them here and start it off as paused
	
	playerSpeed_ = DEFAULT_PLAYER_SPEED;

	//add the remote player shadow
	rem_player_shadow_ = smgr_->addMeshSceneNode(smgr_->getMesh("models/shadows/shadow.ms3d"));
	rem_player_shadow_->setMaterialTexture(0,driver_->getTexture("models/shadows/shadow1.png"));
	rem_player_shadow_->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	//rem_player_shadow_->setScale(vector3df(.85,.85,.85));

	IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/bigfont.png");
	vector3df textPosition = vector3df(characterModel_->getPosition().X, characterModel_->getPosition().Y + 10, characterModel_->getPosition().Z);


	nameDisplay_ = smgr_->addBillboardTextSceneNode(font,name_.c_str(),0, dimension2d<f32>(12,2),
		vector3df(characterModel_->getPosition().X, 
		characterModel_->getPosition().Y+19,
		characterModel_->getPosition().Z),-1,video::SColor(255,0,250,0),video::SColor(255,255,0,0));

	//////////////////////////////////////////////////////////////////////////
	//characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,1);
	//characterModel_->setMaterialFlag(video::EMF_LIGHTING,0);
	//characterModel_->setID(ID_IsNotPickable);//so we cannot select our model
	//characterModel_->addShadowVolumeSceneNode();//make realtime shadows on the character
	//////////////////////////////////////////////////////////////////////////
	
	idle();//make the player start at idle

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

void remotePlayer::setposition(irr::core::vector3df pos)
{
	 character_->warp(pos); 
	 characterModel_->setPosition(character_->getWorldTransform().getTranslation());//set the player at the correct pos
	 rem_player_shadow_->setPosition(vector3df(getPosition().X,getPosition().Y-13.0f,getPosition().Z));
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



*/
void remotePlayer::idle()
{
	characterModel_->setFrameLoop(206,250);
}

void remotePlayer::drawName()
{
	//draw the remote players name 
	nameDisplay_->setText(name_.c_str());
	nameDisplay_->setPosition(vector3df(characterModel_->getPosition().X, 
		characterModel_->getPosition().Y+15,
		characterModel_->getPosition().Z));

}
void remotePlayer::setName(const std::string& name)
{
	name_="<";//prefix
	for(size_t i=0;i<name.size();++i)
	{
		name_+= (wchar_t)name[i];

	}
	name_+=">";//postfix 
	//create < player_name >

}