
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
#include<vector>

#include "common.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include"kinematiccharactercontroller.h"
#include"exampleframework.h"
//#include "NPC.h"
#include "remotePlayer.h"



//Player Variables/////////////
#define DEFAULT_PLAYER_SPEED 3.0f
#define CAMERA_DISTANCE_BACK 30.0f
#define CAMERA_OFFSET 15.0f
#define CAMERA_MOVE_SPEED 50.0f
#define PLAYER_JUMP_FORCE 60.0f
#define PLAYER_EARTH_GRAVITY 150.0f
#define PLAYER_ANIMATION_SPEED 12.0f
#define PLAYER_HEALTH 100
#define MAX_CLUES 3
#define CLUES_TO_FIND 1
#define REACH_DIST 100.0f
///////////////////////////////////


class player
{

public:

	player(irr::IrrlichtDevice* device,char* filename,
		irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
		irrklang::ISoundEngine* engine, irrBulletWorld* world, std::string player_name,int model_type);
	
	
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
	void damage		   ();
	void kill();
	void setState(int state)					  { current_state_ = state; }

	const irr::f32 getXDir () const		{ return xDirection_; }
	const irr::f32 getZDir () const		{ return zDirection_; }
	bool isOnGround        ()           { return character_->isOnGround();  }
	bool isEnemyInRange	   ()			{ return enemyInRange_; }
	bool enemyNPCInRange   ()			{ return enemyNPCInRange_; }
	bool enemyPlayerInRange()			{ return enemyPlayerInRange_; }
	bool isClueInRange	   ()			{ return clueInRange_; }
	bool isDead()						{ return (current_state_ == DEAD || current_state_ == SPECTATOR); }
	void isAttackingSomeone(bool& attacking, std::string& enemyName);
	irr::scene::ISceneNode* getSelectedNode(){ return selectedSceneNode_; }
	void addClue (ClueObject* clue);
				  

	irr::scene::IAnimatedMeshSceneNode* getPSceneNode () { return characterModel_; }
	irr::core::vector3df                getPosition   () { return characterModel_->getPosition(); }
	irr::core::vector3df                getRotation   () { return characterModel_->getRotation(); }
	irr::scene::IAnimatedMeshSceneNode* getPlayerNode () { return characterModel_; }
	float                               getPlayerSpeed() { return playerSpeed_; }
	char*                               getChatMessage() { return chat_message_; }
	irr::scene::ICameraSceneNode*       getCamera     () { return camera_; }
	int                                 getHealth     () { return playerHealth_; }
	//playerData&                         getPlayerData () {return data_;}
	int                                 get_model_type() { return player_model_type_;}
	int                                 get_curr_state() { return current_state_;}
	bool                                getIsLamp     () { return lamp_->isVisible(); }
	std::string                         getname       () { return player_name_; }
	std::vector<ClueObject*>			getFoundClues () { return clues_; }
	bool								isKillerKnown () { return (clues_.size() >= CLUES_TO_FIND); }
	const irr::c8*						getSelectedNodeName() { return selectedSceneNode_->getName(); }	
	const int&							getCurrentState(){ return current_state_; }

	//directional 
	void                  forward   ();
	void                  backwards ();
	void                  left      ();
	void                  right     ();
	void                  idle      ();
	void                  attack    ();

	bool isStepSoundPaused() { return playerSteps_->getIsPaused(); }
	void setStepSoundPaused(bool setting) { playerSteps_->setIsPaused(setting); }

private:
	IKinematicCharacterController* character_;
	irr::scene::IAnimatedMeshSceneNode* characterModel_;
	irr::scene::IMeshSceneNode* player_shadow_;

    irr::scene::ICameraSceneNode* camera_;
	irr::scene::ICameraSceneNode* fpsCam_;

	irrBulletWorld* world_;
	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	
	irrklang::ISoundEngine* engine_;
	irrklang::ISound* playerSteps_;
	irrklang::ISound* player_punch_;
	irrklang::ISound* player_jump_;
	
	irr::u32 then;

	irr::f32 xDirection_;
	irr::f32 zDirection_;

	irr::f32 mouseCursorX_;
	irr::f32 mouseCursorY_;

	float playerSpeed_;
	char* chat_message_;
	int playerHealth_;
	int walkframe_;
	
	irr::scene::ISceneNode* selectedSceneNode_;

	//playerData data_;
	float cameradist_;

	irr::scene::ILightSceneNode* lamp_;
	std::string player_name_;
	
	bool enemyInRange_;
	bool enemyNPCInRange_;
	bool enemyPlayerInRange_;
	bool clueInRange_;
	int player_model_type_;	
	int current_state_;
	std::vector<ClueObject*> clues_;
};
