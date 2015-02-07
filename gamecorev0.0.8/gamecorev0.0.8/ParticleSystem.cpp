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

	smgr_(smgr),driver_(driver),box_(box),direction_(direction),maxStartAmount_(maxStartAmount),
	minStartAmount_(minStartAmount),startColor_(startColor),endColor_(endColor),maxLifetime_(maxLifetime),
	minLifetime_(minLifetime),maxAngle_(maxAngle),startSize_(startSize),endSize_(endSize)
{
	particleNode_ = smgr_->addParticleSystemSceneNode(false);
}

ParticleSystem::~ParticleSystem(void)
{
}
