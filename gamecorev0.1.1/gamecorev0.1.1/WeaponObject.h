#pragma once
#include "InteractiveObject.h"
#include <irrlicht.h>
#include <string>
#include "common.h"

class WeaponObject : public InteractiveObject
{
public:
	WeaponObject(std::string filename, irr::IrrlichtDevice* device, irr::core::vector3df pos);
	
	virtual ~WeaponObject(void);

	virtual const irr::core::string<char> getObjectType() const {return type_; }
	virtual void update();

	virtual void eraseModelFromWorld();

private:

	irr::IrrlichtDevice* device_;
	std::string file_;
	std::string skin_;
	irr::scene::IMeshSceneNode* weapon_;
	irr::core::string<char> type_;

};
