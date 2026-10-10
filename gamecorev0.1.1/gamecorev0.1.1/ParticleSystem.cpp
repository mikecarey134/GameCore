//////////////////////////////////////////////////////////////////////////
//Particle.cpp
//Class to incorporate custom particle systems &&
//simplifies the process
//by Michael Carey
//////////////////////////////////////////////////////////////////////////


#include "ParticleSystem.h"


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


ParticleSystem::ParticleSystem(ISceneManager* smgr,IVideoDriver* driver,
	const aabbox3df& box,vector3df direction,u32 maxStartAmount,u32 minStartAmount, 
	SColor startColor,SColor endColor, u32 maxLifetime,u32 minLifetime,
	u32 maxAngle, dimension2df startSize,dimension2df endSize):

	box_(box),startColor_(startColor),endColor_(endColor),maxStartAmount_(maxStartAmount),minStartAmount_(minStartAmount),
	maxLifetime_(maxLifetime),minLifetime_(minLifetime),maxAngle_(maxAngle),startSize_(startSize),
	endSize_(endSize),direction_(direction),driver_(driver),smgr_(smgr)
{
	particleNode_ = smgr_->addParticleSystemSceneNode(false);
}

ParticleSystem::~ParticleSystem(void)
{
}
