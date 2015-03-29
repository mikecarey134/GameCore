#include "mapLoader.h"
#include "common.h"

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
	device_->getSceneManager()->loadScene(mapName_);
	driver_->setFog(SColor(0,255,255,255),EFT_FOG_EXP2,200,400,.001,true,false);

	array<ISceneNode*> nodes;
	smgr_->getSceneNodesFromType(ESNT_ANY,nodes);
	ITriangleSelector* selector = 0;

	for (u32 i=0; i < nodes.size(); ++i)//create collision response and psychics for our scene nodes
	{

		ISceneNode* node = nodes[i];
		stringc name = node->getName();
		const stringc prefix = name.subString(0,name.findFirst('_'));
		const stringc suffix = name.subString(name.findFirst('_')+1, name.size()-name.findFirst('_'));


		if(node->getType() == ESNT_MESH || ESNT_CUBE || ESNT_SPHERE || ESNT_ANY)
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

				world_->addRigidBody(shape);

				node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
			}

			else if(prefix == "static")
			{
				IBvhTriangleMeshShape* shape = new IBvhTriangleMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), 0.0f);
				IRigidBody* body = world_->addRigidBody(shape);
				node->setMaterialFlag(EMF_FOG_ENABLE,true);//add fog to our scene
				//node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
				//node->setID(IDFlag_IsPickable);

				//selector = smgr_->createTriangleSelectorFromBoundingBox(node);
				//node->setTriangleSelector(selector);

			}

			else if(prefix == "soft")
			{
				ISoftBody* softbody = world_->addSoftBody(static_cast<IMeshSceneNode*>(node));
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
		//node->setMaterialFlag(EMF_LIGHTING,0);
		node->setAutomaticCulling(EAC_BOX);//cull unneeded primitives 
	
		vector3df clueLocation(167, -4, 67);
		clueLocations_.push_back(clueLocation);
		clueLocation = vector3df(114, -4, 80);
		clueLocations_.push_back(clueLocation);
	}
}

void mapLoader::closeMap()
{

}