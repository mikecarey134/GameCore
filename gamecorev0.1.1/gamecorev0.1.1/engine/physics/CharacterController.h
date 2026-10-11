#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include <vector3d.h>

struct CharacterSettings
{
	float radius = 2.5f;
	float height = 4.5f;
	float stepHeight = 0.35f;
	float maxSlope = 0.785398f;
	float gravity = 29.4f;
	float jumpSpeed = 10.0f;
	bool collidesWithCharacters = true;
};

class CharacterController
{
public:
	virtual ~CharacterController() {}

	virtual void warp(const irr::core::vector3df& position) = 0;
	virtual irr::core::vector3df position() const = 0;
	virtual void setWalk(const irr::core::vector3df& unitsPerSecond) = 0;
	virtual void jump() = 0;
	virtual bool onGround() const = 0;
	virtual void setGravity(float gravity) = 0;
	virtual void setJumpSpeed(float speed) = 0;
	virtual void setMaxSlope(float radians) = 0;
};

#endif
