#include "OrbitCamera.h"
#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <irrMath.h>
#include <cmath>

OrbitCamera::OrbitCamera(irr::scene::ISceneManager* smgr)
	: yaw_(0.0f), pitch_(0.0f), distance_(10.0f), sensitivity_(0.0625f), smgr_(smgr), node_(0)
{
	if (smgr_)
	{
		node_ = smgr_->addCameraSceneNode();
		node_->bindTargetAndRotation(true);
	}
}

void OrbitCamera::makeActive()
{
	if (node_)
		smgr_->setActiveCamera(node_);
}

void OrbitCamera::update(const irr::core::position2df& mouseDelta)
{
	yaw_ += mouseDelta.X * sensitivity_;
	pitch_ = irr::core::clamp(pitch_ - mouseDelta.Y * sensitivity_, -89.9f, 39.9f);
	if (node_)
	{
		node_->setPosition(position());
		node_->setTarget(target_);
	}
}

void OrbitCamera::zoom(float delta)
{
	distance_ = irr::core::clamp(distance_ + delta, 5.0f, 50.0f);
}

irr::core::vector3df OrbitCamera::position() const
{
	return target_ - lookDirection() * distance_;
}

irr::core::vector3df OrbitCamera::lookDirection() const
{
	const float yaw = yaw_ * irr::core::DEGTORAD;
	const float pitch = pitch_ * irr::core::DEGTORAD;
	return irr::core::vector3df(cosf(yaw), sinf(pitch), -sinf(yaw));
}
