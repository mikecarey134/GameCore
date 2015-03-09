#include "ClueObject.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

ClueObject::ClueObject(irr::core::string<char> filename, IrrlichtDevice* device, irr::scene::ISceneManager* smgr,
					   irrBulletWorld* world, irr::video::IVideoDriver* driver, mapLoader* theMap) : 
						device_(device), smgr_(smgr), world_(world), driver_(driver), theMap_(theMap)
{
	type_ = "Clue";
	prepareClue();

	model_ = smgr->addMeshSceneNode(smgr->getMesh(filename),0,-1,clueProperties_.location_,
									vector3df(90.0f, 0.0f, 0.0f));
	
	model_->setMaterialFlag(EMF_LIGHTING,0);

	model_->setID(player::IDFlag_IsPickable);
	model_->setName("Clue");

	//clueCollisionObject_ = new IBoxShape(model_, model_->getBoundingBox().getVolume()*(f32)0.001);

	//world_->addRigidBody(clueCollisionObject_);
	model_->setMaterialFlag(EMF_BACK_FACE_CULLING,true);

	//model_->setPosition(clueProperties_.location_);

	model_->setScale(vector3df(3,3,3)*model_->getScale());
	//clueCollisionObject_->getSceneNode()->setPosition(clueProperties_.location_);
	//model_->setRotation(vector3df(90.0f, 0.0f, 0.0f));

	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(model_);
	model_->setTriangleSelector(selector);
	
	std::cout << clueProperties_.location_.X << " " << clueProperties_.location_.Y << " " << clueProperties_.location_.Z << std::endl;
}

void ClueObject::prepareClue()
{
	clueProperties_.location_ = setLocation();
	clueProperties_.clueMessage_ = setClueMsg();

	//clueProperties_ = properties(msg, loc);
}

vector3df ClueObject::setLocation()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);

	luaL_dofile(L, "scripts/ClueSetup.lua");

	lua_getglobal(L, "pickRandomLocation");

	lua_pushinteger(L, theMap_->getClueLocations().size());

	if (lua_pcall(L,1,1,0) != EXIT_SUCCESS)
		exit(1);

	int selection = lua_tointeger(L, -1);

	return theMap_->getClueLocations()[selection];
}

string<char> ClueObject::setClueMsg()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);

	luaL_dofile(L, "scripts/ClueSetup.lua");

	lua_getglobal(L, "setupClueMsg");

	if (lua_pcall(L,0,1,0) != EXIT_SUCCESS)
		exit(1);

	string<char> msg = lua_tostring(L, -1);

	return msg;
}