//////////////////////////////////////////////////////////////////////////
//Particle.h 
//Class to incorporate particle systems into the gamecore
//by Michael Carey
//////////////////////////////////////////////////////////////////////////

#pragma once
#include<irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class ParticleSystem
{
public:

	ParticleSystem::ParticleSystem(ISceneManager* smgr,IVideoDriver* driver,
		const aabbox3df& box = aabbox3df(-10,28,-10,10,30,10),		   
		vector3df direction = vector3df(0.0f,0.1f,0.0f),
		u32 maxStartAmount = 100,u32 minStartAmount = 50, SColor startColor = SColor(0,255,0,0),
		SColor endColor = SColor(0,252,248,18), u32 maxLifetime= 800,u32 minLifetime = 500,
		u32 maxAngle = 0, dimension2df startSize= dimension2df(5.0f,5.0f),
		dimension2df endSize= dimension2df(8.0f,8.0f));

	~ParticleSystem(void);
	//set up where you want the particle system to be
	void setPosition   (irr::core::vector3df pos)     {particleNode_->setPosition(pos);}
	//set the scale of the created particle system
	void setScale      (irr::core::vector3df scalar) {particleNode_->setScale(scalar);}
	
	void setUpMaterials(char* file_path)//set up the texture of the particle system
	{
		particleNode_->setMaterialFlag(irr::video::EMF_LIGHTING,false);//no lighting applied to the texture
		particleNode_->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE,false);
		particleNode_->setAutomaticCulling(EAC_OFF);
		particleNode_->setMaterialTexture(0,driver_->getTexture(file_path));
		particleNode_->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);

	}
	//init the emitter 
	void setUpEmitter()
	{
	
		emitter_ = particleNode_->createBoxEmitter(box_,direction_,minStartAmount_,
		maxStartAmount_,startColor_,endColor_,minLifetime_,maxLifetime_,maxAngle_,startSize_,endSize_);
		particleNode_->setEmitter(emitter_);
		emitter_->drop();
	
	}
	void hide(bool b_val){particleNode_->setVisible(!b_val);}//hides or shows the system
														     //inversed to make calls like hide(true) make sense

private:

	irr::core::aabbox3df box_;
	irr::video::SColor startColor_;
	irr::video::SColor endColor_;

	irr::u32 maxStartAmount_;
	irr::u32 minStartAmount_;

	irr::u32 maxLifetime_;
	irr::u32 minLifetime_;

	irr::u32 maxAngle_;

	irr::core::dimension2df startSize_;
	irr::core::dimension2df endSize_;
	
	irr::core::vector3df direction_;

	irr::video::IVideoDriver* driver_;
	irr::scene::ISceneManager* smgr_;
	irr::scene::IParticleSystemSceneNode* particleNode_;
	irr::scene::IParticleEmitter* emitter_;
	

};
