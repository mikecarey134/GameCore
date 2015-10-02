/************************************************************************/
/*	SolarCycle
	purpose: add the sun and moon to the game to make a dynamic light system
	by: Michael Carey
*/
/************************************************************************/

#pragma once
#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>

class SolarCycle
{
public:
	SolarCycle(irr::IrrlichtDevice* device,irr::core::vector3df location);
	~SolarCycle(void);
	void set_position(irr::core::vector3df location)
	{
		location_= location; solar_entity_->setPosition(location_);
		solar_emitter_->setPosition(location_);
	}
	irr::core::vector3df get_position(){return location_;}

private:
	irr::scene::IMeshSceneNode* solar_entity_;
	irr::scene::ILightSceneNode* solar_emitter_;
	irr::core::vector3df location_;
};
