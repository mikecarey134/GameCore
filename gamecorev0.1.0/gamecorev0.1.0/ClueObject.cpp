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
	type_ = TYPE_CLUE;
	prepareClue();

	model_ = smgr->addMeshSceneNode(smgr->getMesh(filename),0,-1,clueProperties_.location_,
									vector3df(90.0f, 0.0f, 0.0f));
	
	model_->setMaterialFlag(EMF_LIGHTING,0);

	model_->setID(IDFlag_IsPickable);

	//clueCollisionObject_ = new IBoxShape(model_, model_->getBoundingBox().getVolume()*(f32)0.001);

	//world_->addRigidBody(clueCollisionObject_);
	model_->setMaterialFlag(EMF_BACK_FACE_CULLING,true);

	//model_->setPosition(clueProperties_.location_);

	model_->setScale(vector3df(3,3,3)*model_->getScale());
	//clueCollisionObject_->getSceneNode()->setPosition(clueProperties_.location_);
	//model_->setRotation(vector3df(90.0f, 0.0f, 0.0f));

	model_->setName(TYPE_CLUE);

	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(model_);
	model_->setTriangleSelector(selector);

	std::cout << clueProperties_.location_.X << " " << clueProperties_.location_.Y << " " << clueProperties_.location_.Z << std::endl;
}

void ClueObject::prepareClue()
{
	clueProperties_.location_ = setLocation();
	clueProperties_.clueMessage_ = setClueMsg();
	clueProperties_.isFound_ = false;
	clueProperties_.showClue_ = false;

	//clueProperties_ = properties(msg, loc);
}

void ClueObject::update()
{ 
	if (!clueProperties_.isFound_)
	{
		model_->setRotation(irr::core::vector3df(model_->getRotation().X, model_->getRotation().Y+1, model_->getRotation().Z));
	}
	else
		drawClueMessage();
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

void ClueObject::drawClueMessage() 
{	
	int posX = 500;
	int posY = 100;
	int width = 250;
	int height = 400;

	int textHeigt = 50;
	int charWidth = 15;
	int textOffset = 40;

	IGUIFont* font = device_->getGUIEnvironment()->getFont("bill/console.bmp");//device_->getGUIEnvironment()->getBuiltInFont();

	//clueProperties_.showClue_ = true;
	ITexture* note = driver_->getTexture("textures/bloodyNote.png");
	
	driver_->draw2DImage(note,position2d<s32>(posX,posY),rect<s32>(0,0,width,height),NULL,SColor(255,255,255,255), true);

	int lineStart = 0;
	int lineLength = 20;
	int i = lineLength;
	int j = 0;
	string<char> line;
	while (i < clueProperties_.clueMessage_.size())
	{
		j = 0;	
		if (clueProperties_.clueMessage_[i] != ' ')
		{					
			while ((i+j < clueProperties_.clueMessage_.size()) && clueProperties_.clueMessage_[i-j] != ' ')
				++j;
		}
		
		line = clueProperties_.clueMessage_.subString(lineStart,lineLength);
		lineStart += lineLength-j;
		i += lineLength;
		posY += textOffset;
		
		font->draw(line, 
				   rect<s32>(posX+textOffset, posY+textOffset, 
							 posX+(line.size()*charWidth), posY+textOffset+textHeigt),
				   SColor(255,0,0,0));
	}
	//Draw the last line, since the quits before it prints. 
	//Might want to find a better way to do this
	line = clueProperties_.clueMessage_.subString(lineStart+1, clueProperties_.clueMessage_.size()-lineStart);
	posY += textOffset;
	font->draw(line, 
		rect<s32>(posX+textOffset, posY+textOffset, 
		posX+(line.size()*charWidth), posY+textOffset+textHeigt),
		SColor(255,0,0,0));
}

void ClueObject::eraseModelFromWorld()
{
	model_->remove();
	clueProperties_.isFound_ = true;
}