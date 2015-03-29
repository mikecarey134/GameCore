/************************************************************************/
/* remotePlayer.h
	purpose: to render client characters and animate them
	By: Michael Carey
*/
/************************************************************************/



#pragma once
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>
#include <string>

#include<string>
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"
#include"common.h"

class remotePlayer
{
public:
	remotePlayer(){}
	remotePlayer(irr::IrrlichtDevice* device,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		irrklang::ISoundEngine* engine, irrBulletWorld* world, int model_type);

	~remotePlayer(void);

	void                 setState           (int state);
	void                 jump				();
	void                 attack				();
	void                 rotate				(irr::core::vector3df rotation){ characterModel_->setRotation(rotation); }
	void                 animate			(irr::scene::EMD2_ANIMATION_TYPE animation);
	void                 moveCameraControl	();
	irr::core::vector3df calculateCameraPos	();
	void				 nodeSelector		();
	void                 delete_player      (){characterModel_->remove(); rem_player_shadow_->remove();
												delete character_; nameDisplay_->remove(); }
    void                 drawName();
	
	void setRotation(irr::core::vector3df pos)    {characterModel_->setRotation(pos);}
	void setTexture    (const char* filename)     {characterModel_->setMaterialTexture(0,driver_->getTexture(filename));}
	void setposition   (irr::core::vector3df pos);
	void setGravity    (irr::f32 newGravity)      {character_->setGravity(newGravity);}
	void setSpeed      (float newSpeed)           {playerSpeed_ = newSpeed;}
	void setXDir       (irr::f32 dir)             { xDirection_ = dir; }
	void setZDir       (irr::f32 dir)             { zDirection_ = dir; }
	void setName       (const std::string& name);         

	const irr::f32 getXDir () const		{ return xDirection_; }
	const irr::f32 getZDir () const		{ return zDirection_; }
	bool isOnGround        ()           {return character_->isOnGround();  }

	irr::scene::IAnimatedMeshSceneNode* getPSceneNode () {return characterModel_;}
	irr::core::vector3df                getPosition   () {return characterModel_->getPosition();}
	irr::core::vector3df                getRotation   () {return characterModel_->getRotation();}
	irr::scene::IAnimatedMeshSceneNode* getPlayerNode () { return characterModel_; }
	float                               getPlayerSpeed() {return playerSpeed_;}
	int                                 getHealth     () {return playerHealth_;}
	

private:
	

	void idle();
	IKinematicCharacterController* character_;
	irr::scene::IAnimatedMeshSceneNode* characterModel_;
	irr::scene::IMeshSceneNode* rem_player_shadow_;

	irrBulletWorld* world_;
	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;

	irrklang::ISoundEngine* engine_;
	irrklang::ISound* playerSteps_;
	irrklang::ISound* player_punch;


	irr::u32 then;

	irr::f32 xDirection_;
	irr::f32 zDirection_;

	irr::f32 mouseCursorX_;
	irr::f32 mouseCursorY_;

	float playerSpeed_;
	char* chat_message_;
	int playerHealth_;
	irr::scene::IBillboardTextSceneNode* nameDisplay_;
	irr::core::stringw name_;
	common_paths paths_;


};
