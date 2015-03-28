#pragma once

#include<irrlicht.h>
#include<irrKlang.h>
#include<irrbullet.h>

class InteractiveObject
{
public:
	InteractiveObject () {};
	virtual ~InteractiveObject();

	virtual const irr::core::string<char> getObjectType() const = 0;
	virtual void update() = 0;
	virtual void eraseModelFromWorld() = 0;
};
