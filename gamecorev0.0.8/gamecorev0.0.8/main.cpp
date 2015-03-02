/************************************************************************/
/*
	Main.cpp
	Purpose: The main game-loop for "The Uninvited" v0.0.8
	By:Michael Carey && Dan Brown
*/
/************************************************************************/

#define _IRR_WINDOWS_

#define CONSOLE//debugging console
#include "exampleframework.h"
#include <irrbullet.h>
#include <irrlicht.h>
#include <irrKlang.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "driverChoice.h"
#include "SAppcontext.h"
#include "keymap.h"
#include "console.h"
#include "consoleevent.h"
#include "player.h"
#include "NPC.h"
#include "ParticleSystem.h"
#include "InternalServer.h"
#include "mapLoader.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include "Config_Reader.h"	
#include <winsock2.h>




//standard irrlicht namespaces
using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace RakNet;

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#pragma comment(lib, "ws2_32.lib")
#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "irrKlang.lib")

#ifdef CONSOLE//if we want the console displayed
#else
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")//comment out if you want to see the Debug console
#pragma comment(lib,"IrrConsole-DEBUG")
#endif
#endif



int main(void)
{
	srand(time(NULL));
	//setup the irrlicht devices here
	IrrlichtDevice *device = createDevice(EDT_OPENGL,//standard open_gl drivers
	dimension2d<u32>(800,600),16,false,true,true);
	device->setWindowCaption(L"gamecore v0.0.8");//set the caption for the window
	
	Config_Reader config_reader(device,"config/configure.txt");
	ISoundEngine* engine = createIrrKlangDevice();//add sounds to irrlicht
	IVideoDriver* driver = device->getVideoDriver();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	GUI theGui;theGui.drawloading(driver,guienv);//load gui and display loading while the computer is working
	
	//load our scene manager and set up the shadow color with an alpha channel
	ISceneManager* smgr = device->getSceneManager();smgr->setShadowColor(irr::video::SColor(70,0,0,0));
	keymap userkeys;

	//////////////////////////////////////////////////////////////////////////
	//Alpha Particle class Tester
	//////////////////////////////////////////////////////////////////////////
	ParticleSystem particle_system(smgr,driver);
	particle_system.setPosition(vector3df(-13,48,-296));
	particle_system.setUpMaterials("textures/brightfire.jpg");
	particle_system.setUpEmitter();

	ParticleSystem blood_particle_system(smgr,driver,irr::core::aabbox3df(-10,48,-10,10,30,10),vector3df(0.0f,0.0f,0.1f),178,45
		,SColor(100,90,67,184),SColor(150,252,8,215),1000,200,20,dimension2df(0.2,0.2),dimension2df(10.0,10.0));
	blood_particle_system.setPosition(vector3df(-22,-35,166));
	blood_particle_system.setUpMaterials("textures/particle_red.bmp");
	blood_particle_system.setUpEmitter();
	//////////////////////////////////////////////////////////////////////////
	

	//set up the bullet physics here for our world
	irrBulletWorld* world;//the irrbullet world 
	world = createIrrBulletWorld(device, true, true);
	world->setDebugMode(EPDM_DrawAabb | EPDM_DrawContactPoints);
	world->setGravity(vector3df(0,-20,0));

	SAppContext context;//guiEvent stuff
	context.device = device;
	context.counter = 0;
	
	//load our player into the scene
	player thePlayer(device,"characters/stick_mike.ms3d",smgr,driver, engine, world,config_reader.get_player_name());
	NPC npc_tester(device,smgr,world,driver);

	if(!device)//if device fails to load exit
	{
		return 1;
	}
	if (!engine)//if the irrklang device fails exit 
	{
		return 1;
	}
	
	InternalServer Client(true,device,"characters/stick_mike.ms3d",smgr,driver,engine,world,config_reader.get_ip(),
							config_reader.get_rem_port(), config_reader.get_port());

	driver->setFog(SColor(0,255,255,255),EFT_FOG_EXP2,200,400,.001,true,false);	

	//Loading map now handled in its own class
	mapLoader currentMap(device,driver,smgr,world);
	currentMap.setMap("subbs.irr");
	//currentMap->setMap("arena.irr");
	//currentMap->setMap("dungeon2.irr");
	
		//takes care of all input devices and events calls update during the gameloop
	consoleevent crecv(device,guienv,driver,context,theGui, engine, &thePlayer, world, &npc_tester, &currentMap);//event handeler
	device->setEventReceiver(&crecv);

	gui::IGUIFont* font2 = device->getGUIEnvironment()->getFont("bill/bigfont.png");

	currentMap.loadMap();

	IneractiveObject* testClue;
	testClue = new ClueObject("models/cluePrototype.obj",device,smgr,world,driver,&currentMap);

	crecv.addInteractiveObject(testClue);

	npc_tester.setID(player::selectionType::IDFlag_IsPickable);

	u32 then = device->getTimer()->getTime();

	ISound* intro = engine->play2D("sounds/slowintro.mp3", true);	

	while(device->run())//while the game is running
	{	
	
	
		const u32 now = device->getTimer()->getTime();
		const u32 frameDeltaTime = now - then;	// Time in milliseconds
		then = now;

		
		
		if (device->isWindowActive())//if a window is running
		{	
				
				
				if (!crecv.getStarted())
				{
					driver->beginScene(true,true,video::SColor(0,0,0,0));//begin scene with a white background
					

					if (!engine->isCurrentlyPlaying("sounds/slowintro.mp3"))
						engine->play2D("sounds/slowintro.mp3", true);//startup the main menu track
					
					crecv.drawMainMenu();

				
				}
				else
				{
					//if we have started our game
					if(crecv.getStarted()){
					if (!engine->isCurrentlyPlaying("sounds/man.mp3"))
						engine->play2D("sounds/man.mp3", true);//startup the main track				
					}

					//internal client sends data to the server
	
					Client.messageLoop(thePlayer,theGui,guienv->getBuiltInFont());
					driver->beginScene(true,true,video::SColor(0,0,0,0));//begin scene with a white background

					if (engine->isCurrentlyPlaying("sounds/slowintro.mp3"))//if we are starting
							engine->removeSoundSource("sounds/slowintro.mp3");//remove the main menu sounds

					//driver->beginScene(true,true,video::SColor(0,0,0,0));//begin scene with a white background

					//make cursor temp visible until a better menu is in place
						if (!crecv.getPaused() && !crecv.getIsInventory())
						{
							device->getCursorControl()->setVisible(false);
							device->getCursorControl()->setPosition( 0.5f, 0.5f );
						}
						else
							device->getCursorControl()->setVisible(true);

						
						smgr->drawAll();
					

						world->stepSimulation(frameDeltaTime*0.001f, 120);//bullet psysics
						crecv.update(then,now);//update our game here

						//we dont want the camera to update while the player is stationary
						if (!crecv.getPaused() && !crecv.getIsInventory()&& !crecv.getIsConsole())
						{
							thePlayer.moveCameraControl();
							testClue->update();

							if(!npc_tester.isDead())
								npc_tester.moveNPC();//move our npc 
	
							crecv.playerNpcCollisionCheck();
						}
						
					
						//GUI CLASS calls
						if(font2)
						{
							gui::IGUIFont* font = device->getGUIEnvironment()->getBuiltInFont();
							//draw the current gui related items
							theGui.drawHealth(font2,core::rect<s32>(10,565,450,100),thePlayer.getHealth());
							//theGui.drawCrosshair(font2,core::rect<s32>(400,300,450,100));
							//theGui.drawMessage(font,irr::core::rect<irr::s32>(325,275,475,325),"");
						}
					
					crecv.displayconsole(frameDeltaTime);
					
				
					
				}
				
			
			driver->endScene();

		}
		else
			device->yield();
	}

	//reclaim memory and shutdown device
	device->drop();



	if(Client.isOnline())//if we are online
	{
		Client.shutdown(thePlayer);//shutdown internal server
	}
	return 0;
}




