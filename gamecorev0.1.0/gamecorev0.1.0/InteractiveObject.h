#pragma once

#include<irrlicht.h>

class IneractiveObject
{
public:
	IneractiveObject () {}
	virtual ~IneractiveObject() {}

	virtual const irr::core::string<char> getObjectType() const = 0;
	virtual void update() const = 0;
};