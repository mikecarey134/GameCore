#include "console/Stdafx.h"
#include "defaultCmds.h"
#include <irrlicht.h>


using namespace irr;
IC_Command_TP::IC_Command_TP(player* thePlayer) : IC_Command(L"tp")
{
	thePlayer_= thePlayer;

	setUsage(L"tp <X>,<Y>,<Z>");

	addDescLine("warps player <X><Y><Z>");
}
IC_Command_TP::~IC_Command_TP()
{
}

//warp the player to a specified XYZ Coord
bool IC_Command_TP::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	//if we have the right amount of arguments
	if(args.size() == DEFAULT_ARGS )
	{
		//arguments
		const wchar_t* argx = args[0].c_str();
		const wchar_t* argy = args[1].c_str();
		const wchar_t *argz = args[2].c_str();
		//conversion to coords
		double posx=_wtof(argx),posy=_wtof(argy),posz=_wtof(argz);
		
		//handle 
		//warp the player
		thePlayer_->setposition(irr::core::vector3df((f32)posx,(f32)posy,(f32)posz));
		
		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Arguments! <X><Y><Z>");
}

IC_Command_RB::IC_Command_RB(player* thePlayer,irrBulletWorld* theWorld,irr::IrrlichtDevice* device) : IC_Command(L"rb")
{
	player_ = thePlayer;
	theWorld_ = theWorld;
	device_ = device;

	setUsage("/rb <Filename> <Scale>");

	addDescLine("adds a rigid body to the world");
}
IC_Command_RB::~IC_Command_RB()
{
}

//Add a rigid body into the world for testing
bool IC_Command_RB::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{

	if(args.size() == 2)
	{
		irr::scene::IMeshSceneNode* node;
		const wchar_t* scale = args[1].c_str();
		ICollisionShape* shape;
		node = device_->getSceneManager()->addMeshSceneNode(device_->getSceneManager()->getMesh(args[0]));
		node->setScale(vector3df((f32)_wtof(scale),(f32)_wtof(scale),(f32)_wtof(scale)));
		node->setPosition(vector3df(player_->getPosition().X-10,player_->getPosition().Y,player_->getPosition().Z-10));
		//gpu intensive//////////////////
		node->addShadowVolumeSceneNode();
		/////////////////////////////////
		irr::scene::ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(node);
		node->setTriangleSelector(selector);

		node->setID(/*player::*/IDFlag_IsPickable);
		shape = new IBoxShape(node, node->getBoundingBox().getVolume()*(irr::f32)0.001);
		
		theWorld_->addRigidBody(shape);
		
	}
	else//otherwise display an error
		throw IC_Error(L"Missing Argument! <Filename> <Scale>");

	//pOutput->clearMessages();
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Toggle the worlds gravity
//////////////////////////////////////////////////////////////////////////
IC_Command_GV::IC_Command_GV(irrBulletWorld* world) : IC_Command(L"gv")
{
	world_ = world;

	setUsage("/gv <gravity>");
	addDescLine("sets the world's gravity");
}
IC_Command_GV::~IC_Command_GV()
{
}
bool IC_Command_GV::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	if (args.size()==1)
	{
		world_->setGravity(vector3df(0,(f32)_wtof(args[0].c_str()),0));
	}
	else
		throw IC_Error(L"Missing Argument! <Gravity>");

	return true;
}

IC_Command_PGV::IC_Command_PGV(player* thePlayer) : IC_Command(L"pgv")
{
	thePlayer_=thePlayer;

	setUsage("/pgv <Player Gravity>");
	addDescLine("sets the player's gravity");
}
IC_Command_PGV::~IC_Command_PGV()
{
}
//////////////////////////////////////////////////////////////////////////
//set the players gravity ALPHA State
//////////////////////////////////////////////////////////////////////////
bool IC_Command_PGV::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	if (args.size()==1)
	{
		thePlayer_->setGravity((f32)_wtof(args[0].c_str()));
	}
	else
		throw IC_Error(L"Missing Argument! <Player Gravity>");

	return true;
}

IC_Command_SAY::IC_Command_SAY(player* thePlayer) : IC_Command(L"say")
{
	thePlayer_=thePlayer;

	setUsage("/say <Message>");
	addDescLine("Add Chat Message");
}
IC_Command_SAY::~IC_Command_SAY()
{
}
//////////////////////////////////////////////////////////////////////////
//set the players chat message
//////////////////////////////////////////////////////////////////////////
bool IC_Command_SAY::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	if (args.size()>=1)
	{
		//copy all args to alleviate having to put "" around the message
		core::stringw message;	
		for(u32 i = 0;i < args.size();++i)
		{
			message+= args[i];
			message +=" ";
	
		}
		chatText_.clear();
		for(u32 i = 0; i + 1 < message.size(); ++i)
			chatText_ += (char)message[i];

		thePlayer_->setChatMessage(&chatText_[0]);//set players chat message to the passed in string
		
		//delete [] dest_msg;
	}
	else
		throw IC_Error(L"Missing Argument! <Message>");

	return true;
}


IC_Command_C::IC_Command_C(player* thePlayer) : IC_Command(L"c")
{
	thePlayer_=thePlayer;

	setUsage("/c <Message>");
	addDescLine("Add Chat Message");
}
IC_Command_C::~IC_Command_C()
{
}
//////////////////////////////////////////////////////////////////////////
//simple chat message
//////////////////////////////////////////////////////////////////////////
bool IC_Command_C::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	if (args.size()>=1)
	{
		//copy all args to alleviate having to put "" around the message
		core::stringw message;	
		for(u32 i = 0;i < args.size();++i)
		{
			message+= args[i];
			message +=" ";

		}
		chatText_.clear();
		for(u32 i = 0; i + 1 < message.size(); ++i)
			chatText_ += (char)message[i];

		thePlayer_->setChatMessage(&chatText_[0]);//set players chat message to the passed in string

		//delete [] dest_msg;
	}
	else
		throw IC_Error(L"Missing Argument! <Message>");

	return true;
}


//////////////////////////////////////////////////////////////////////////
//set player speed
IC_Command_P_SPEED::IC_Command_P_SPEED(player* thePlayer) : IC_Command(L"pspeed")
{
	thePlayer_= thePlayer;

	setUsage(L"\\pspeed <New Speed>");

	addDescLine("Increases or decreases player speed");
}
IC_Command_P_SPEED::~IC_Command_P_SPEED()
{
}

//change the player speed for debugging
bool IC_Command_P_SPEED::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		const wchar_t* arg = args[0].c_str();
		//conversion to float speed
		float speed=(float)_wtof(arg);

		//handle 
		thePlayer_->setSpeed(speed);

		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Argument! <New Speed>");
}
//////////////////////////////////////////////////////////////////////////
//set player health
IC_Command_PH::IC_Command_PH(player* thePlayer) : IC_Command(L"ph")
{
	thePlayer_= thePlayer;

	setUsage(L"\\ph <New Health>");

	addDescLine("Increases or decreases player health");
}
IC_Command_PH::~IC_Command_PH()
{
}

//change the player speed for debugging
bool IC_Command_PH::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		const wchar_t* arg = args[0].c_str();
		//conversion to float speed
		float health=(float)_wtof(arg);

		//handle 
		thePlayer_->setHealth((int)health);

		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Argument! <New Health>");
}

IC_Command_LM::IC_Command_LM(mapLoader* currentMap, player* thePlayer) : IC_Command(L"lm"), thePlayer_(thePlayer)
{
	currentMap_ = currentMap;

	setUsage(L"\\lm <New Map>");

	addDescLine("Change the currently loaded map");
}

IC_Command_LM::~IC_Command_LM()
{
}

bool IC_Command_LM::invoke(const array<WideString>& args, IC_Dispatcher*, IC_MessageSink*)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		if (args.size()==1)
		{
			std::string newMap;
			for(unsigned int i=0;i<args[0].size();++i)
				newMap += (char)args[0][i];
			
			currentMap_->closeMap();
			currentMap_->setMap(PREFIX + newMap);
			currentMap_->loadMap();
			std::vector<irr::core::vector3df> spawns = currentMap_->getSpawnPoints();
			thePlayer_->spawn(spawns[rand() % spawns.size()]);
		}
		else
			throw IC_Error(L"Missing Argument! <Message>");
		
		

		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Argument! <New Health>");
}

//////////////////////////////////////////////////////////////////////////
//hurt the current player (Debugging Purposes)
IC_Command_H::IC_Command_H(player* thePlayer) : IC_Command(L"h")
{
	thePlayer_= thePlayer;

	setUsage(L"\\h ");

	addDescLine("Hurt Current Player");
}
IC_Command_H::~IC_Command_H()
{
}

//change the player speed for debugging
bool IC_Command_H::invoke(const array<WideString>&, IC_Dispatcher*, IC_MessageSink*)
{
	thePlayer_->damage();
	return true;
}

void registerGameCommands(IC_Console& console, player* thePlayer, irrBulletWorld* world, irr::IrrlichtDevice* device, mapLoader* currentMap)
{
	console.registerCommand(new IC_Command_TP(thePlayer));
	console.registerCommand(new IC_Command_RB(thePlayer, world, device));
	console.registerCommand(new IC_Command_GV(world));
	console.registerCommand(new IC_Command_PGV(thePlayer));
	console.registerCommand(new IC_Command_P_SPEED(thePlayer));
	console.registerCommand(new IC_Command_PH(thePlayer));
	console.registerCommand(new IC_Command_SAY(thePlayer));
	console.registerCommand(new IC_Command_C(thePlayer));
	console.registerCommand(new IC_Command_H(thePlayer));
	console.registerCommand(new IC_Command_LM(currentMap, thePlayer));
}
