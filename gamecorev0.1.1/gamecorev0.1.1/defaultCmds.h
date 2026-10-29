#ifndef _IrrConsole_Default_Cmds_h_
#define _IrrConsole_Default_Cmds_h_
#include "includes.h"
#include "utils.h"
#include "common.h"
#include "console.h"
#include "player.h"
#include "physics/PhysicsWorld.h"
#include "GameMap.h"
#include "LuaVM.h"

//
//	Default Command Set
//
//warp command class
class IC_Command_TP : public IC_Command
{
	enum{DEFAULT_ARGS = 3};
public:
	IC_Command_TP(player* thePlayer);
	virtual ~IC_Command_TP();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;

};

//add rigid body class
class IC_Command_RB : public IC_Command
{
	enum{DEFAULT_ARGS = 3};
public:
	IC_Command_RB(player* thePlayer,PhysicsWorld* physics,irr::IrrlichtDevice* device);
	virtual ~IC_Command_RB();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* player_;
	PhysicsWorld* physics_;
	irr::IrrlichtDevice* device_;

};
//set the worlds gravity class
class IC_Command_GV : public IC_Command
{
public:
	IC_Command_GV(PhysicsWorld* physics);
	virtual ~IC_Command_GV();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	PhysicsWorld* physics_;

};

//set the players gravity class !!!!!!!!ALPHA STATE
class IC_Command_PGV : public IC_Command
{
public:
	IC_Command_PGV(player* thePlayer);
	virtual ~IC_Command_PGV();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;

};


class IC_Command_P_SPEED : public IC_Command
{
public:
	IC_Command_P_SPEED(player* thePlayer);
	virtual ~IC_Command_P_SPEED();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;
};

class IC_Command_PH : public IC_Command
{
public:
	IC_Command_PH(player* thePlayer);
	virtual ~IC_Command_PH();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;
};

class IC_Command_SAY : public IC_Command
{
public:
	IC_Command_SAY(player* thePlayer);
	virtual ~IC_Command_SAY();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;
	std::string chatText_;
};


class IC_Command_C : public IC_Command
{
public:
	IC_Command_C(player* thePlayer);
	virtual ~IC_Command_C();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;
	std::string chatText_;
};


class IC_Command_H : public IC_Command
{
public:
	IC_Command_H(player* thePlayer);
	virtual ~IC_Command_H();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* thePlayer_;
};



class IC_Command_LM : public IC_Command
{
public:
	IC_Command_LM(mapLoader* currentMap, player* thePlayer);
	virtual ~IC_Command_LM();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	mapLoader* currentMap_;
	player* thePlayer_;
};


class IC_Command_LUA : public IC_Command
{
public:
	IC_Command_LUA(LuaVM* lua);
	virtual ~IC_Command_LUA();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	LuaVM* lua_;
};

void registerGameCommands(IC_Console& console, player* thePlayer, PhysicsWorld* physics, irr::IrrlichtDevice* device, mapLoader* currentMap);

#endif