#pragma once
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>


class GrassNode
{
public:
	GrassNode(irr::IrrlichtDevice* device,irr::core::vector3df location);
	~GrassNode(void);
	void set_position(irr::core::vector3df location)
	{
		location_= location; grass_entity_->setPosition(location_);
	
	}
	irr::core::vector3df get_position(){return location_;}

private:
	irr::scene::IMeshSceneNode* grass_entity_;
	irr::core::vector3df location_;

};
