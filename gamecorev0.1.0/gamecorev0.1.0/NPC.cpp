/************************************************************************/
/* NPC.cpp
	purpose: To make a functional npc for the game
	By: Dan Brown
*/
/************************************************************************/


#include"NPC.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

NPC::NPC(IrrlichtDevice* device, /*char* filename,*/ irr::scene::ISceneManager* smgr,
		 irrBulletWorld* world, irr::video::IVideoDriver* driver) : 
			device_(device), smgr_(smgr), driver_(driver), world_(world),npcHealth_(NPC_HEALTH),
				isLoaded_(NOT_LOADED),isHit_(false),isDead_(false)
{
	character_ = new IKinematicCharacterController(world_);
	//character_->setUseGhostSweepTest(true);
	
	character_->setGravity(NPC_EARTH_GRAVITY);//set the NPC gravity for jumping
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
	directionCounter_ = 0.0f;
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
	IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
	healthText_ = "";
	healthText_ += npcHealth_;
	vector3df textPosition = vector3df(characterModel_->getPosition().X, characterModel_->getPosition().Y + 10, characterModel_->getPosition().Z);

	healthDisplay_->remove();
	healthDisplay_ = smgr_->addBillboardTextSceneNode(font, healthText_.c_str(),0, dimension2d<f32>(11,5),
													  vector3df(characterModel_->getPosition().X, 
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
		directionCounter_ = 0.0f;
	}
	//std::cout << AIdirection_.X << " " << AIdirection_.Z << std::endl;

	characterModel_->setPosition(character_->getWorldTransform().getTranslation());

	character_->setPositionIncrementPerSimulatorStep(AIdirection_* NPC_SPEED);

	drawNPCHealth();
}

void NPC::luaSetDir()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);

	luaL_dofile(L, "scripts/pickDir.lua");

	lua_getglobal(L, "setDirection");

	if (lua_pcall(L,0,3,0) != EXIT_SUCCESS)
		exit(1);

	AIdirection_.X = lua_tonumber(L, -3);
	AIdirection_.Z = lua_tonumber(L, -2);
	vector3df newRotation = vector3df(characterModel_->getRotation().X, lua_tonumber(L, -1), characterModel_->getRotation().Z);
	characterModel_->setRotation(newRotation);
}

void NPC::damage()
{
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

void NPC::loadModel()
{
	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh("characters/stick_dan.ms3d"),
		0, /*player::*/IDFlag_IsPickable);
	characterModel_->setMaterialTexture(0,driver_->getTexture("characters/playerskin_d1.bmp"));
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,1);
	characterModel_->setID(IDFlag_IsPickable);

	//characterModel_->addShadowVolumeSceneNode();
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));	
	//character_->warp(vector3df(30.0f, 105.0f, 23.0f));//set the init pos on the map
	character_->warp(vector3df(259.0f, 38.0f, -25.0f));//set the init pos on the map

	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(characterModel_);
	characterModel_->setTriangleSelector(selector);
	//smgr_->createTriangleSelector(characterModel_);

	characterModel_->setName("test_npc");
	healthText_ += npcHealth_;
	healthDisplay_ = smgr_->addBillboardTextSceneNode(device_->getGUIEnvironment()->getBuiltInFont(),
		healthText_.c_str());

	isLoaded_ = DONE_LOADING;
}