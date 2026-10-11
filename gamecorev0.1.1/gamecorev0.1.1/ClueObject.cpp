#include "ClueObject.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

ClueObject::ClueObject(irr::core::string<char> filename, IrrlichtDevice* device, irr::scene::ISceneManager* smgr,
					   irrBulletWorld*, irr::video::IVideoDriver* driver, GameMap* theMap, LuaVM& lua) : 
						device_(device), driver_(driver), theMap_(theMap), lua_(lua) 
{
	type_ = TYPE_CLUE;
	lua_.doFile("scripts/ClueSetup.lua");
	prepareClue();

	model_ = smgr->addMeshSceneNode(smgr->getMesh(filename),0,-1,clueProperties_.location_,
									vector3df(90.0f, 0.0f, 0.0f));
	
	model_->setMaterialFlag(EMF_LIGHTING,0);

	model_->setID(IDFlag_IsPickable);

	//clueCollisionObject_ = new IBoxShape(model_, model_->getBoundingBox().getVolume()*(f32)0.001);

	//world_->addRigidBody(clueCollisionObject_);
	//model_->setMaterialFlag(EMF_BACK_FACE_CULLING,true);

	//model_->setPosition(clueProperties_.location_);

	model_->setScale(vector3df(3,3,3)*model_->getScale());
	//clueCollisionObject_->getSceneNode()->setPosition(clueProperties_.location_);
	//model_->setRotation(vector3df(90.0f, 0.0f, 0.0f));

	model_->setName(TYPE_CLUE);
	if (theMap_->getClueLocations().empty())
		model_->setVisible(false);

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

void ClueObject::placeOnMap()
{
	if (clueProperties_.isFound_)
		return;
	clueProperties_.location_ = setLocation();
	model_->setPosition(clueProperties_.location_);
	model_->setVisible(!theMap_->getClueLocations().empty());
}

void ClueObject::reset()
{
	clueProperties_.isFound_ = false;
	clueProperties_.showClue_ = false;
	placeOnMap();
}

void ClueObject::update()
{ 
	if (!clueProperties_.isFound_)
	{
		model_->setRotation(irr::core::vector3df(model_->getRotation().X, model_->getRotation().Y+1, model_->getRotation().Z));
	}
	else if (clueProperties_.showClue_)
		drawClueMessage();
}


vector3df ClueObject::setLocation()
{
	int selection = 0;
	std::vector<LuaValue> results;
	if (lua_.call("pickRandomLocation", { LuaValue((int)theMap_->getClueLocations().size()) }, results, 1))
		selection = results[0].asInt();

	const std::vector<vector3df> locations = theMap_->getClueLocations();
	if (locations.empty())
		return vector3df(0,0,0);
	if (selection < 0 || selection >= (int)locations.size())
		selection = 0;
	return locations[selection];
}

string<char> ClueObject::setClueMsg()
{
	string<char> msg = "";
	std::vector<LuaValue> results;
	if (lua_.call("setupClueMsg", {}, results, 1) && results[0].isString())
		msg = results[0].asString().c_str();

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
	while (i < (int)clueProperties_.clueMessage_.size())
	{
		j = 0;	
		if (clueProperties_.clueMessage_[i] != ' ')
		{					
			while ((i+j < (int)clueProperties_.clueMessage_.size()) && clueProperties_.clueMessage_[i-j] != ' ')
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
	model_->setVisible(false);
	clueProperties_.isFound_ = true;
	clueProperties_.showClue_ = true;
}