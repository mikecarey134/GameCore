/************************************************************************/
/* NPC.cpp
	purpose: To make a functional npc for the game
	By: Dan Brown
*/
/************************************************************************/


#include"NPC.h"
#include <iostream>

using namespace irr;
using namespace gamecore;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

NPC::NPC(IrrlichtDevice* device, /*char* filename,*/ irr::scene::ISceneManager* smgr,
		 PhysicsWorld* physics, irr::video::IVideoDriver* driver, LuaVM& lua) : 
			device_(device), smgr_(smgr), driver_(driver), physics_(physics), lua_(lua),npcHealth_(NPC_HEALTH),
				isLoaded_(NOT_LOADED),isHit_(false),isDead_(false)
{
	CharacterSettings settings;
	settings.gravity = NPC_EARTH_GRAVITY;
	character_ = physics_->addCharacter(settings);
	lua_.doFile("scripts/pickDir.lua");
	/*
	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh("characters/stick_dan.ms3d"),
		0, IDFlag_IsPickable);
	characterModel_->setMaterialTexture(0,driver_->getTexture("characters/playerskin_d1.bmp"));
    characterModel_->setMaterialFlag(video::EMF_LIGHTING,1);
	
	//characterModel_->addShadowVolumeSceneNode();
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));	
	//character_->warp(vector3df(30.0f, 105.0f, 23.0f));//set the init pos on the map
	character_->warp(vector3df(259.0f, 38.0f, -25.0f));//set the init pos on the map
	*/
	AIdirection_ = vector3df(0.0f,0.0f,0.0f);
	directionCounter_ = 0;
	/*
	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(characterModel_);
	characterModel_->setTriangleSelector(selector);
	//smgr_->createTriangleSelector(characterModel_);

	characterModel_->setName("test_npc");
	healthText_ += npcHealth_;
	healthDisplay_ = smgr_->addBillboardTextSceneNode(device_->getGUIEnvironment()->getBuiltInFont(),
													 healthText_.c_str());
													 */
}

NPC::~NPC()
{

}

void NPC::drawNPCHealth()
{
	healthText_ = "";
	healthText_ += npcHealth_;

	healthDisplay_->setText(healthText_.c_str());
	healthDisplay_->setSize(dimension2d<f32>(11,5));
	healthDisplay_->setPosition(vector3df(characterModel_->getPosition().X, 
										  characterModel_->getPosition().Y+19,
										  characterModel_->getPosition().Z));
	
	if (npcHealth_ > 50)
		healthDisplay_->setColor(SColor(255,0,255,0));
	else if (npcHealth_ <= 50 && npcHealth_ > 20)
		healthDisplay_->setColor(SColor(255,255,252,66));
	else
		healthDisplay_->setColor(SColor(255,255,0,0));
	
}

void NPC::moveNPC()
{
	++directionCounter_;
	if (directionCounter_ > 100)
	{
		//luaSetDir();
		directionCounter_ = 0;
	}
	//std::cout << AIdirection_.X << " " << AIdirection_.Z << std::endl;

	characterModel_->setPosition(character_->position());

	character_->setWalk(AIdirection_* NPC_SPEED * 60.0f);

	drawNPCHealth();
}

void NPC::luaSetDir()
{
	std::vector<LuaValue> results;
	if (!lua_.call("setDirection", {}, results, 3))
		return;

	AIdirection_.X = (f32)results[0].asNumber();
	AIdirection_.Z = (f32)results[1].asNumber();
	vector3df newRotation = vector3df(characterModel_->getRotation().X, (f32)results[2].asNumber(), characterModel_->getRotation().Z);
	characterModel_->setRotation(newRotation);
}

void NPC::despawn()
{
	if (isLoaded_ == DONE_LOADING)
	{
		characterModel_->remove();
		if (!isDead_)
			healthDisplay_->remove();
	}
	isLoaded_ = NOT_LOADED;
}

void NPC::reset()
{
	despawn();
	isDead_ = false;
	npcHealth_ = NPC_HEALTH;
}

void NPC::damage()
{
	if (isDead_)
		return;
	npcHealth_ -= 5;
	std::cout << npcHealth_ << std::endl;

	if (npcHealth_ < 1)
	{
		characterModel_->setFrameLoop(166,166);//166-173);
		//characterModel_->setCurrentFrame(173);
		
		characterModel_->setPosition(vector3df(1000,1000,1000));		
		character_->warp(vector3df(1000,1000,1000));
		//characterModel_->setVisible(false);
		isDead_ = true;
		healthDisplay_->remove();

		//characterModel_->drop();
	}
}

void NPC::loadModel(vector3df spawn)
{
	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh("characters/stick_dan.ms3d"),
		0, /*player::*/IDFlag_IsPickable);
	characterModel_->setMaterialTexture(0,driver_->getTexture("characters/playerskin_d1.bmp"));
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,1);
	characterModel_->setID(IDFlag_IsPickable);

	//characterModel_->addShadowVolumeSceneNode();
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));	
	//character_->warp(vector3df(30.0f, 105.0f, 23.0f));//set the init pos on the map
	character_->warp(spawn);//set the init pos on the map

	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(characterModel_);
	characterModel_->setTriangleSelector(selector);
	//smgr_->createTriangleSelector(characterModel_);

	characterModel_->setName("test_npc");
	healthText_ += npcHealth_;
	healthDisplay_ = smgr_->addBillboardTextSceneNode(device_->getGUIEnvironment()->getBuiltInFont(),
		healthText_.c_str());

	isLoaded_ = DONE_LOADING;
}