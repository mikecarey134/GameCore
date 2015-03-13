#include "InteractiveObject.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
/*
InteractiveObject::InteractiveObject()(irr::core::string<char> filename, IrrlichtDevice* device, irr::scene::ISceneManager* smgr,
									   irrBulletWorld* world, irr::video::IVideoDriver* driver, mapLoader* theMap) : 
		device_(device), smgr_(smgr), world_(world), driver_(driver), theMap_(theMap)
{
	model_ = smgr->addMeshSceneNode(smgr->getMesh(filename),0,-1,clueProperties_.location_,
		vector3df(90.0f, 0.0f, 0.0f));

	model_->setMaterialFlag(EMF_LIGHTING,0);

	model_->setID(player::IDFlag_IsPickable);

	model_->setMaterialFlag(EMF_BACK_FACE_CULLING,true);

	model_->setScale(vector3df(3,3,3)*model_->getScale());

	model_->setName("clue");
	model_->setID(10);

	ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(model_);
	model_->setTriangleSelector(selector);
}
*/
InteractiveObject::~InteractiveObject()
{
}