#include "GrassNode.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

GrassNode::GrassNode(irr::IrrlichtDevice* device,irr::core::vector3df location)
{

	grass_entity_ = device->getSceneManager()->addMeshSceneNode(device->getSceneManager()->getMesh("models/nature/grass/grass.md2"));
	grass_entity_->setMaterialFlag(EMF_LIGHTING,1);
	
	
	
	grass_entity_->setPosition(location_);

	grass_entity_->setMaterialTexture(0,device->getVideoDriver()->getTexture("models/nature/grass/grass.png"));
	grass_entity_->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
	grass_entity_->setMaterialFlag(EMF_BACK_FACE_CULLING ,0);
	grass_entity_->setScale(vector3df(0.06,0.04,0.06));
}

GrassNode::~GrassNode(void)
{
}
