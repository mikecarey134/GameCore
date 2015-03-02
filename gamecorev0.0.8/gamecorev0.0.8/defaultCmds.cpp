#include "console/Stdafx.h"
#include "defaultCmds.h"
#include <irrlicht.h>


using namespace irr;
IC_Command_ECHO::IC_Command_ECHO() : IC_Command(L"echo")
{
	setUsage(L"echo <string>");
	addDescLine(L"This command echoes the given string to console");
}
IC_Command_ECHO::~IC_Command_ECHO()
{
}
bool IC_Command_ECHO::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if(args.size() > 1)
	{
		WideString wstr = args[0];
		for(u32 i = 1; i < args.size(); i++)
		{
			wstr += L" ";
			wstr += args[i];
		}
		pOutput->appendMessage(wstr);
	}
	return true;
}
IC_Command_HELP::IC_Command_HELP() : IC_Command(L"help")
{
	setUsage(L"help <cmd-name>");
	addDescLine(L"this command prints the help available for console commands");
	addDescLine(L"if cmd-name is not supplied a list of commands is printed with usage");
	addDescLine(L"any command has to be preceded with the backslash character to execute");
	addDescLine(L"e.g. \\help");
}
IC_Command_HELP::~IC_Command_HELP()
{
}
bool IC_Command_HELP::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if(args.size() == 0)
	{
		pDispatcher->printCommandList(pOutput,true);
	}
	else
	{
		WideString wstr = args[0];
		for(u32 i = 1; i < args.size(); i++)
		{
			wstr += L" ";
			wstr += args[i];
		}
		if(pDispatcher->hasCommand(wstr))
		{
			pDispatcher->printCommandDesc(wstr,pOutput);
		}
		else
		{
			WideString msg = " No help available for command ";
			msg+= wstr;
			pOutput->appendMessage(msg);
		}
	}
	return true;
}

IC_Command_LIST::IC_Command_LIST(): IC_Command(L"list")
{
	setUsage("list <detailed>");
	addDescLine("This command lists the available commands");
	addDescLine("If an additional paramter is specified then");
	addDescLine("the command returns usage information for the commands");
}
IC_Command_LIST::~IC_Command_LIST()
{
}
bool IC_Command_LIST::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if(args.size() > 0)
	{
		pDispatcher->printCommandList(pOutput,true);
	}
	else
	{
		pDispatcher->printCommandList(pOutput,false);
	}
	return true;
}


IC_Command_DRIVER_INFO::IC_Command_DRIVER_INFO(irr::IrrlichtDevice *pDevice) : IC_Command("driver_info"),device(pDevice)
{
	setUsage("driver_info");
	addDescLine("This command prints some info about the engine");

}
IC_Command_DRIVER_INFO::~IC_Command_DRIVER_INFO()
{
	device = 0;
}
bool IC_Command_DRIVER_INFO::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if(device)
	{
		irr::video::IVideoDriver* driver = device->getVideoDriver();
		WideString wstr = L" Irrlicht Version : ";
		wstr+= device->getVersion();
		pOutput->appendMessage(wstr);
/*
		wstr = L" OS Version : ";
		wstr+= device->getOSOperator()->getOperationSystemVersion();
		pOutput->appendMessage(wstr);
*/
		wstr = L" Display Driver : ";
		wstr+= device->getVideoDriver()->getName();
		pOutput->appendMessage(wstr);

		wstr=L"";
		return true;
	}
	else
	{
		throw IC_Error(L"No valid irrlicht device detected!!");
	}
}
IC_Command_EXIT::IC_Command_EXIT() : IC_Command(L"exit")
{
	setUsage("exit");
	addDescLine("sets the console to invisible");
}
IC_Command_EXIT::~IC_Command_EXIT()
{
}
bool IC_Command_EXIT::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	pOutput->toggleVisible();
	return true;
}

IC_Command_QUIT::IC_Command_QUIT() : IC_Command(L"quit")
{
	setUsage("exit");
	addDescLine("sets the console to invisible");
}
IC_Command_QUIT::~IC_Command_QUIT()
{
}
bool IC_Command_QUIT::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	pOutput->toggleVisible();
	return true;
}

IC_Command_CLS::IC_Command_CLS() : IC_Command(L"cls")
{
	//setUsage("clear");
	
	addDescLine("clears the console messages");
}
IC_Command_CLS::~IC_Command_CLS()
{
}
bool IC_Command_CLS::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	//if we are compiling with windows clear the console
    #ifdef _WINDOWS_
		system("cls");
		std::cout<<"Console Clear Command Issued!\n";
	#endif
	

	
	pOutput->clearMessages();
	return true;
}
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
bool IC_Command_TP::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
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
		thePlayer_->setposition(irr::core::vector3df(posx,posy,posz));
		
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
bool IC_Command_RB::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{

	if(args.size() == 2)
	{
		irr::scene::IMeshSceneNode* node;
		const wchar_t* scale = args[1].c_str();
		ICollisionShape* shape;
		node = device_->getSceneManager()->addMeshSceneNode(device_->getSceneManager()->getMesh(args[0]));
		node->setScale(vector3df(_wtof(scale),_wtof(scale),_wtof(scale)));
		node->setPosition(vector3df(player_->getPosition().X-10,player_->getPosition().Y,player_->getPosition().Z-10));
		//gpu intensive//////////////////
		node->addShadowVolumeSceneNode();
		/////////////////////////////////
		irr::scene::ITriangleSelector* selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(node);
		node->setTriangleSelector(selector);

		node->setID(player::IDFlag_IsPickable);
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
bool IC_Command_GV::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if (args.size()==1)
	{
		world_->setGravity(vector3df(0,_wtof(args[0].c_str()),0));
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
bool IC_Command_PGV::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if (args.size()==1)
	{
		thePlayer_->setGravity(_wtof(args[0].c_str()));
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
bool IC_Command_SAY::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	if (args.size()==1)
	{
		char* message = new char[args[0].size()];//dynamically allocate a c string and copy the contents of args[0]
		unsigned int i=0;
		for(;i<args[0].size();++i)
		{
			message[i] = (char)args[0][i];
		
		}

		message[i] ='\0';//insert null terminator

		thePlayer_->setChatMessage(message);//set players chat message to the passed in string
		//delete [] message;//deallocate dynamic memory
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

	setUsage(L"\pspeed <New Speed>");

	addDescLine("Increases or decreases player speed");
}
IC_Command_P_SPEED::~IC_Command_P_SPEED()
{
}

//change the player speed for debugging
bool IC_Command_P_SPEED::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		const wchar_t* arg = args[0].c_str();
		//conversion to float speed
		float speed=_wtof(arg);

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

	setUsage(L"\ph <New Health>");

	addDescLine("Increases or decreases player health");
}
IC_Command_PH::~IC_Command_PH()
{
}

//change the player speed for debugging
bool IC_Command_PH::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		const wchar_t* arg = args[0].c_str();
		//conversion to float speed
		float health=_wtof(arg);

		//handle 
		thePlayer_->setHealth(health);

		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Argument! <New Health>");
}

IC_Command_LM::IC_Command_LM(mapLoader* currentMap) : IC_Command(L"lm")
{
	currentMap_ = currentMap;

	setUsage(L"\lm <New Map>");

	addDescLine("Change the currently loaded map");
}

IC_Command_LM::~IC_Command_LM()
{
}

bool IC_Command_LM::invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput)
{
	//if we have the right amount of arguments
	if(args.size() == 1)
	{
		//arguments
		const wchar_t* arg = args[0].c_str();
		if (args.size()==1)
		{
			char* newMap = new char[args[0].size()];//dynamically allocate a c string and copy the contents of args[0]
			unsigned int i=0;
			for(;i<args[0].size();++i)
			{
				newMap[i] = (char)args[0][i];

			}

			newMap[i] ='\0';//insert null terminator
			
			currentMap_->closeMap();
			currentMap_->setMap(newMap);
			currentMap_->loadMap();
		}
		else
			throw IC_Error(L"Missing Argument! <Message>");
		
		

		return true;
	}
	else//otherwise display an error
		throw IC_Error(L"No Valid Argument! <New Health>");
}