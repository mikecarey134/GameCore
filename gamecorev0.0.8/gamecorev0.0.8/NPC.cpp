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
			device_(device), smgr_(smgr), driver_(driver), world_(world),npcHealth_(NPC_HEALTH),isHit_(false)
{
	character_ = new IKinematicCharacterController(world_);
	//character_->setUseGhostSweepTest(true);
	
	character_->setGravity(NPC_EARTH_GRAVITY);//set the NPC gravity for jumping
	
	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh("characters/stick_dan.ms3d"),
		0, player::IDFlag_IsPickable);
	characterModel_->setMaterialTexture(0,driver_->getTexture("characters/playerskin_d1.bmp"));
    characterModel_->setMaterialFlag(video::EMF_LIGHTING,1);
	
	//characterModel_->addShadowVolumeSceneNode();
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));	
	//character_->warp(vector3df(30.0f, 105.0f, 23.0f));//set the init pos on the map
	character_->warp(vector3df(259.0f, 38.0f, -25.0f));//set the init pos on the map
	AIdirection_ = vector3df(0.0f,0.0f,0.0f);
	directionCounter_ = 0.0f;
	
	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(characterModel_);
	characterModel_->setTriangleSelector(selector);
	//smgr_->createTriangleSelector(characterModel_);
}

NPC::~NPC()
{

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
	if (npcHealth_ <= 0)
	{
		characterModel_->setFrameLoop(166,173);//166-173);
		//characterModel_->setCurrentFrame(173);
		characterModel_->remove();
	}
	else
	{
		npcHealth_ -= 5;
	}
}