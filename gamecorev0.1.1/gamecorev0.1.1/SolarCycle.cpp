//************************************************************************
/*	
	SolarCycle
	purpose: add the sun and moon to the game to make a dynamic light system
	by: Michael Carey
*/
//************************************************************************

#include "SolarCycle.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

SolarCycle::SolarCycle(IrrlichtDevice* device, vector3df location):location_(location)
{
	
	solar_entity_ = device->getSceneManager()->addMeshSceneNode(device->getSceneManager()->getMesh("models/nature/sun.md2"));
	solar_entity_->setScale(vector3df(3,3,3));
	solar_entity_->setMaterialFlag(EMF_LIGHTING,0);
	solar_emitter_= device->getSceneManager()->addLightSceneNode(0,vector3df(0,0,0),video::SColorf(1.0f,1.0f,1.0f),2000);
	solar_entity_->setPosition(location_);
	solar_emitter_->setPosition(solar_entity_->getPosition());
	solar_entity_->setMaterialTexture(0,device->getVideoDriver()->getTexture("models/nature/suns.jpg"));
	

}

SolarCycle::~SolarCycle(void)
{
}
