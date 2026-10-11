#include "mapLoader.h"
#include <vector>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


mapLoader::mapLoader (IrrlichtDevice* device, IVideoDriver* driver, ISceneManager* smgr, irrBulletWorld* world): 
device_(device), driver_(driver), smgr_(smgr), world_(world)
{
	mapName_ = "";
}

void mapLoader::loadMap()
{
	core::list<ISceneNode*> before = smgr_->getRootSceneNode()->getChildren();

	device_->getSceneManager()->loadScene(mapName_);
	//driver_->setFog(SColor(0,255,255,255),EFT_FOG_EXP2,200,400,.001,true,false);



	array<ISceneNode*> nodes;
	smgr_->getSceneNodesFromType(ESNT_ANY,nodes);

	for (u32 i=0; i < nodes.size(); ++i)//create collision response and psychics for our scene nodes
	{

		ISceneNode* node = nodes[i];
		node->updateAbsolutePosition();
		stringc name = node->getName();
		const stringc prefix = name.subString(0,name.findFirst('_'));
		const stringc suffix = name.subString(name.findFirst('_')+1, name.size()-name.findFirst('_'));


		if(prefix == "spawn")
			spawnPoints_.push_back(node->getAbsolutePosition());

		ESCENE_NODE_TYPE type = node->getType();
		if(type == ESNT_MESH || type == ESNT_OCTREE || type == ESNT_CUBE || type == ESNT_SPHERE)
		{
			if(prefix == "rigid")
			{
				ICollisionShape* shape = 0;

				if(suffix == "mesh")
					shape = new IGImpactMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), node->getBoundingBox().getVolume()*(irr::f32)0.001);

				else if(suffix == "box")
					shape = new IBoxShape(node, node->getBoundingBox().getVolume()*(f32)0.001);

				else if(suffix == "sphere")
					shape = new ISphereShape(node, node->getBoundingBox().getVolume()*(f32)0.001);

				if(shape)
					bodies_.push_back(world_->addRigidBody(shape));

				node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
			}

			else if(prefix == "static")
			{
			
				IBvhTriangleMeshShape* shape = new IBvhTriangleMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), 0.0f);
				bodies_.push_back(world_->addRigidBody(shape));
				//node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
				//node->setID(IDFlag_IsPickable);
	


				//selector = smgr_->createTriangleSelectorFromBoundingBox(node);
				//node->setTriangleSelector(selector);

			}

			else if(prefix == "soft")
			{
				ISoftBody* softbody = world_->addSoftBody(static_cast<IMeshSceneNode*>(node));
				bodies_.push_back(softbody);
				softbody->setTotalMass(0.1f, false);
				softbody->setActivationState(EAS_DISABLE_DEACTIVATION);
				node->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
				node->setAutomaticCulling(EAC_OFF);

				softbody->getConfiguration().liftCoefficient = 0.0;
				softbody->getConfiguration().dragCoefficient = 0.0;
				softbody->getConfiguration().dampingCoefficient = 0.0;
				softbody->getConfiguration().poseMatchingCoefficient = 0.0f;
				softbody->getConfiguration().positionsSolverIterations = 56;
				softbody->updateConfiguration();
			}


		}
		//node->setDebugDataVisible(EDS_BBOX);
		if(node->getType() != ESNT_SKY_BOX)
			node->setMaterialFlag(EMF_LIGHTING,1);
		node->setAutomaticCulling(EAC_BOX);//cull unneeded primitives 
	}

	device_->getVideoDriver()->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

	onMapLoaded();

	const core::list<ISceneNode*>& after = smgr_->getRootSceneNode()->getChildren();
	for (core::list<ISceneNode*>::ConstIterator it = after.begin(); it != after.end(); ++it)
	{
		bool isNew = true;
		for (core::list<ISceneNode*>::ConstIterator old = before.begin(); old != before.end(); ++old)
			if (*old == *it)
				isNew = false;
		if (isNew)
		{
			(*it)->grab();
			mapNodes_.push_back(*it);
		}
	}
}

void mapLoader::closeMap()
{
	for (size_t i = 0; i < bodies_.size(); ++i)
	{
		bodies_[i]->includeNodeOnRemoval(false);
		world_->removeCollisionObject(bodies_[i]);
	}
	bodies_.clear();

	for (size_t i = 0; i < mapNodes_.size(); ++i)
	{
		mapNodes_[i]->remove();
		mapNodes_[i]->drop();
	}
	mapNodes_.clear();
	spawnPoints_.clear();
	onMapClosed();
}

std::vector<vector3df> mapLoader::getSpawnPoints() const
{
	if (spawnPoints_.empty())
		return std::vector<vector3df>(1, vector3df(0, 10, 0));
	return spawnPoints_;
}
