
/************************************************************************/
/* Player.h
	purpose: to render the character and animate them
	By: Michael Carey
*/
/************************************************************************/


#pragma once
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>
#include<string>

#include<string>
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"
#include "playerData.h"

//Player Variables/////////////
#define DEFAULT_PLAYER_SPEED 3.0f
#define CAMERA_DISTANCE_BACK 30.0f
#define CAMERA_MOVE_SPEED 50.0f
#define PLAYER_JUMP_FORCE 60.0f
#define PLAYER_EARTH_GRAVITY 150.0f
#define PLAYER_ANIMATION_SPEED 12.0f
#define PLAYER_HEALTH 100
///////////////////////////////////


class player
{

public:

	enum selectionType
	{
		// I use this ISceneNode ID to indicate a scene node that is
		// not pickable by getSceneNodeAndCollisionPointFromRay()
		ID_IsNotPickable = 0,

		// I use this flag in ISceneNode IDs to indicate that the
		// scene node can be picked by ray selection.
		IDFlag_IsPickable = 1 << 0,

		// I use this flag in ISceneNode IDs to indicate that the
		// scene node can be highlighted.  In this example, the
		// hominids can be highlighted, but the level mesh can't.
		IDFlag_IsHighlightable = 1 << 1
	};


	player(irr::IrrlichtDevice* device,char* filename,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		irrklang::ISoundEngine* engine, irrBulletWorld* world);
	
	
	~player(void);

	void                 jump				();
	void                 rotate				(irr::core::vector3df rotation){ characterModel_->setRotation(rotation); }
	void                 animate			(irr::scene::EMD2_ANIMATION_TYPE animation);
	void                 moveCameraControl	();
	irr::core::vector3df calculateCameraPos	();
	void				 nodeSelector		();


	void setTexture    (const char* filename)     {characterModel_->setMaterialTexture(0,driver_->getTexture(filename));}
	void setposition   (irr::core::vector3df pos) { character_->warp(pos); }//set the player at the correct pos
	void setGravity    (irr::f32 newGravity)      {character_->setGravity(newGravity);}
	void setSpeed      (float newSpeed)           {playerSpeed_ = newSpeed;}
	void setXDir       (irr::f32 dir)             { xDirection_ = dir; }
	void setZDir       (irr::f32 dir)             { zDirection_ = dir; }
	void setChatMessage(char* message)            { chat_message_ = message;}
	void setCamDist    (float increment)          {cameradist_+= increment;}
	void setlamp       (bool ison)                {lamp_->setVisible(ison);}
	void setHealth     (int newHealth)            {playerHealth_ = newHealth;}

	const irr::f32 getXDir () const		{ return xDirection_; }
	const irr::f32 getZDir () const		{ return zDirection_; }
	bool isOnGround        ()           {return character_->isOnGround();  }
	bool isEnemyInRange	   ()			{ return enemyInRange_; }

	irr::scene::IAnimatedMeshSceneNode* getPSceneNode () {return characterModel_;}
	irr::core::vector3df                getPosition   () {return characterModel_->getPosition();}
	irr::core::vector3df                getRotation   () {return characterModel_->getRotation();}
	irr::scene::IAnimatedMeshSceneNode* getPlayerNode () { return characterModel_; }
	float                               getPlayerSpeed() {return playerSpeed_;}
	char*                               getChatMessage() {return chat_message_;}
	irr::scene::ICameraSceneNode*       getCamera     () {return camera_;}
	int                                 getHealth     () {return playerHealth_;}
	//playerData&                         getPlayerData () {return data_;}
	bool                                getIsLamp     () {return lamp_->isVisible();}
	std::string                         getname       () {return player_name_;}
	

	//directional 
	void                  forward   ();
	void                  backwards ();
	void                  left      ();
	void                  right     ();
	void                  idle      ();
	void                  attack    (){characterModel_->setFrameLoop(32,44); characterModel_->setAnimationSpeed(15);}


	bool isStepSoundPaused() { return playerSteps_->getIsPaused(); }
	void setStepSoundPaused(bool setting) { playerSteps_->setIsPaused(setting); }


private:


	IKinematicCharacterController* character_;
	irr::scene::IAnimatedMeshSceneNode* characterModel_;
    irr::scene::ICameraSceneNode* camera_;
	irr::scene::ICameraSceneNode* fpsCam_;

	irrBulletWorld* world_;
	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	
	irrklang::ISoundEngine* engine_;
	irrklang::ISound* playerSteps_;
	
	
	irr::u32 then;

	irr::f32 xDirection_;
	irr::f32 zDirection_;

	irr::f32 mouseCursorX_;
	irr::f32 mouseCursorY_;

	float playerSpeed_;
	char* chat_message_;
	int playerHealth_;
	int walkframe_;

	//playerData data_;
	float cameradist_;

	irr::scene::ILightSceneNode* lamp_;
	std::string player_name_;
	
	bool enemyInRange_;
};
