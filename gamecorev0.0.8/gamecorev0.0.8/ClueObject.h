#pragma once

#include "InteractiveObject.h"

#include<irrKlang.h>
#include<irrbullet.h>
#include <lua.hpp>
#include <vector>
#include <iostream>

#include "mapLoader.h"
#include "player.h"

class ClueObject: public IneractiveObject
{
private:
	struct properties
	{
		irr::core::string<char> clueMessage_;
		bool isFound_;
		irr::core::vector3df location_;
	};

public:
	ClueObject(irr::core::string<char> filename, irr::IrrlichtDevice* device, irr::scene::ISceneManager* smgr,
		irrBulletWorld* world, irr::video::IVideoDriver* driver, mapLoader* theMap);
	~ClueObject() {}

	const irr::core::string<char> getObjectType() const { return type_; }
	void update() const { model_->setRotation(irr::core::vector3df(model_->getRotation().X, model_->getRotation().Y+2, model_->getRotation().Z)); }

	void prepareClue();
	irr::core::vector3df setLocation();
	irr::core::string<char> setClueMsg();

private:
	ICollisionShape* clueCollisionObject_;
	irr::scene::ISceneNode* model_;
	properties clueProperties_;
	irr::core::string<char> type_;

	irr::IrrlichtDevice* device_;
	irr::scene::ISceneManager* smgr_;
	irr::video::IVideoDriver* driver_;
	irrBulletWorld* world_;
	mapLoader* theMap_;
};