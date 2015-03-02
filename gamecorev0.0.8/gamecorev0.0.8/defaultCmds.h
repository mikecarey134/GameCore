#ifndef _IrrConsole_Default_Cmds_h_
#define _IrrConsole_Default_Cmds_h_
#include "includes.h"
#include "utils.h"
#include "console.h"
#include "player.h"
#include<irrbullet.h>

//
//	Default Command Set
//
class IC_Command_ECHO : public IC_Command
{
public:
	IC_Command_ECHO();
	virtual ~IC_Command_ECHO();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
};

class IC_Command_HELP : public IC_Command
{
public:
	IC_Command_HELP();
	virtual ~IC_Command_HELP();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
};

class IC_Command_LIST : public IC_Command
{
public:
	IC_Command_LIST();
	virtual ~IC_Command_LIST();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
};

class IC_Command_DRIVER_INFO : public IC_Command
{
public:
	IC_Command_DRIVER_INFO(irr::IrrlichtDevice *pDevice);
	virtual ~IC_Command_DRIVER_INFO();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	irr::IrrlichtDevice *device;
};

class IC_Command_EXIT : public IC_Command
{
public:
	IC_Command_EXIT();
	virtual ~IC_Command_EXIT();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);

};

class IC_Command_QUIT : public IC_Command
{
public:
	IC_Command_QUIT();
	virtual ~IC_Command_QUIT();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);

};

class IC_Command_CLS : public IC_Command
{
public:
	IC_Command_CLS();
	virtual ~IC_Command_CLS();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);

};

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
	IC_Command_RB(player* thePlayer,irrBulletWorld* theWorld,irr::IrrlichtDevice* device);
	virtual ~IC_Command_RB();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	player* player_;
	irrBulletWorld* theWorld_;
	irr::IrrlichtDevice* device_;

};
//set the worlds gravity class
class IC_Command_GV : public IC_Command
{
public:
	IC_Command_GV(irrBulletWorld* world);
	virtual ~IC_Command_GV();
	bool invoke(const array<WideString>& args, IC_Dispatcher* pDispatcher, IC_MessageSink* pOutput);
private:
	irrBulletWorld* world_;

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
};



#endif