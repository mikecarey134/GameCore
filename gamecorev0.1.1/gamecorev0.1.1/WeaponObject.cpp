
/************************************************************************/
/* WeaponObject.cpp
	purpose: character weapons 
	By: Michael Carey
*/
/************************************************************************/


#include "WeaponObject.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


WeaponObject::WeaponObject(std::string filename, IrrlichtDevice* device, vector3df pos):device_(device)
{
	ISceneManager* smgr =device_->getSceneManager();
	weapon_ = smgr->addMeshSceneNode((smgr->getMesh(filename.c_str())),0,-1,pos) ;
	//weapon_->setMaterialTexture(0,device_->getVideoDriver()->getTexture("models/shadows/shadow1.png"));
	type_= TYPE_WEAPON;
}

WeaponObject::~WeaponObject(void)
{
}
void WeaponObject::eraseModelFromWorld(){weapon_->remove();}
void WeaponObject::update(){}