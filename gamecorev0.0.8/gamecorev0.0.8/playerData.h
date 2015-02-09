#ifndef PLAYERDATA_H
#define PLAYERDATA_H


//////////////////////////////////////////////////////////////////////////
/*
	PlayerData.h
	get the player data and send it to the network
	By: Michael Carey
*/
//////////////////////////////////////////////////////////////////////////

#include "RakNetTypes.h"
#include <string>
#include <irrlicht.h>


class playerData
{
public:
	playerData();
	~playerData();
	playerData(const playerData& orig);

	//Direction&           getDirection   ()                       { return myDirection_;}
	irr::core::vector3df getworldCoord  ()                       { return cord_;}
	irr::core::vector3df getcamPosition ()                       { return camPos_;}
	int			         getWeapon      ()                       { return weaponNum_;}
	int			         getState       ()                       { return state_;}
	void		         setGuid        (const std::string guid);
	std::string	         getGuid        ();
	std::istream&        getInput       (std::istream& in);

private:
	//Direction myDirection_;
	irr::core::vector3df cord_;
	irr::core::vector3df camPos_;
	int state_;
	int weaponNum_;
	std::string guid_;
};

std::istream& operator>>(std::istream& in, playerData& ourPlayerData);

#endif