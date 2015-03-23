#pragma once

#include "InteractiveObject.h"

#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>
#include <lua.hpp>
#include <vector>
#include <iostream>

#include "common.h"
#include "mapLoader.h"

class ClueObject: public InteractiveObject
{
private:
	struct properties
	{
		irr::core::string<char> clueMessage_;
		irr::core::vector3df location_;

		bool isFound_;
		bool showClue_;
	};

public:
	ClueObject(irr::core::string<char> filename, irr::IrrlichtDevice* device, irr::scene::ISceneManager* smgr,
				irrBulletWorld* world, irr::video::IVideoDriver* driver, mapLoader* theMap);
	~ClueObject() {}

	const irr::core::string<char> getObjectType() const { return type_; }
	void update(); 
	void prepareClue();
	irr::core::vector3df setLocation();
	irr::core::string<char> setClueMsg();
	irr::scene::ISceneNode* getModel () const { return model_; }
	const bool& isFound() const				  { return clueProperties_.isFound_; }

	void drawClueMessage();

	void eraseModelFromWorld();

private:
	properties clueProperties_;
	irr::core::string<char> type_;
	
	ICollisionShape* clueCollisionObject_;
	irr::scene::ISceneNode* model_;

	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	irrBulletWorld* world_;
	mapLoader* theMap_;
};
