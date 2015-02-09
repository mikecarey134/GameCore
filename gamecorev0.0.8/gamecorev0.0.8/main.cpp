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

#include "driverChoice.h"
#include "SAppcontext.h"
#include "keymap.h"
#include "console.h"
#include "consoleevent.h"
#include "player.h"
#include "NPC.h"
#include "ParticleSystem.h"
#include "InternalServer.h"
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

	//setup the irrlicht devices here
	IrrlichtDevice *device = createDevice(EDT_OPENGL,//standard open_gl drivers
	dimension2d<u32>(800,600),16,false,true,true);
	device->setWindowCaption(L"gamecore v0.0.8");//set the caption for the window
	
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
	player thePlayer(device,"characters/stick_mike.ms3d",smgr,driver, engine, world);
	NPC npc_tester(device,smgr,world,driver);

	if(!device)//if device fails to load exit
	{
		return 1;
	}
	if (!engine)//if the irrklang device fails exit 
	{
		return 1;
	}
	
	InternalServer Client(true);

	
	////Dungeon example ////////////////////////////////////////////////////////
	//device->getSceneManager()->loadScene("map/dungeon2.irr");//debug map scene
	//////////////////////////////////////////////////////////////////////////

	//Open World Example///////////////////////////////////////////////////////
	//device->getSceneManager()->loadScene("map/arena.irr");//debug map scene
	//device->getSceneManager()->loadScene("map/test.irr");//debug map scene
	driver->setFog(SColor(0,255,255,255),EFT_FOG_EXP2,200,400,.001,true,false);
	////////////////////////////////////////////////////////////////////////////
	
	//house example////////////////////////////////////////////////////////////////
	device->getSceneManager()->loadScene("map/subbs.irr");//debug map scene
	//////////////////////////////////////////////////////////////////////////////

	
		//takes care of all input devices and events calls update during the gameloop
		consoleevent crecv(device,guienv,driver,context,theGui, engine, &thePlayer, world, npc_tester);//event handeler
	device->setEventReceiver(&crecv);

	gui::IGUIFont* font2 = device->getGUIEnvironment()->getFont("bill/bigfont.png");

	
	core::array<scene::ISceneNode*> nodes;
	smgr->getSceneNodesFromType(scene::ESNT_ANY,nodes);
	scene::ITriangleSelector* selector = 0;

	for (u32 i=0; i < nodes.size(); ++i)//create collision response and psychics for our scene nodes
	{
	
		scene::ISceneNode* node = nodes[i];
		stringc name = node->getName();
		const stringc prefix = name.subString(0,name.findFirst('_'));
		const stringc suffix = name.subString(name.findFirst('_')+1, name.size()-name.findFirst('_'));


		if(node->getType() == scene::ESNT_MESH || scene::ESNT_CUBE || scene::ESNT_SPHERE ||scene::ESNT_ANY)
		{
			if(prefix == "rigid")
			{
				ICollisionShape* shape = 0;

				if(suffix == "mesh")
					shape = new IGImpactMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), node->getBoundingBox().getVolume()*(irr::f32)0.001);

				else if(suffix == "box")
					shape = new IBoxShape(node, node->getBoundingBox().getVolume()*(irr::f32)0.001);

				else if(suffix == "sphere")
					shape = new ISphereShape(node, node->getBoundingBox().getVolume()*(irr::f32)0.001);

				world->addRigidBody(shape);

				node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
			}

			else if(prefix == "static")
			{
			
				IBvhTriangleMeshShape* shape = new IBvhTriangleMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), 0.0f);
				IRigidBody* body = world->addRigidBody(shape);
				node->setMaterialFlag(EMF_FOG_ENABLE,true);//add fog to our scene
				//node->setMaterialFlag(EMF_BACK_FACE_CULLING,true);
				//node->setID(player::IDFlag_IsPickable);
				
				//selector = smgr->createTriangleSelectorFromBoundingBox(node);
			    //node->setTriangleSelector(selector);
	
			}

			else if(prefix == "soft")
			{
				ISoftBody* softbody = world->addSoftBody(static_cast<IMeshSceneNode*>(node));
				softbody->setTotalMass(0.1f, false);
				softbody->setActivationState(EAS_DISABLE_DEACTIVATION);
				node->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
				node->setAutomaticCulling(EAC_OFF);
			
				softbody->getConfiguration().liftCoefficient = 0.0;
				softbody->getConfiguration().dragCoefficient = 0.0;
				softbody->getConfiguration().dampingCoefficient = 0.0;
				softbody->getConfiguration().poseMatchingCoefficient = 0.0f;
				softbody->getConfiguration().positionsSolverIterations = 56;
				softbody->updateConfiguration();
			}


		}
		//node->setDebugDataVisible(EDS_BBOX);
		//node->setMaterialFlag(EMF_LIGHTING,0);
		node->setAutomaticCulling(EAC_BOX);//cull unneeded primitives 
		
	}

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
							npc_tester.moveNPC();//move our npc 
							crecv.playerNpcCollisionCheck();
						}
						
					
						//GUI CLASS calls
						if(font2)
						{
							gui::IGUIFont* font = device->getGUIEnvironment()->getBuiltInFont();
							//draw the current gui related items
							//theGui.drawHealth(font2,core::rect<s32>(10,565,450,100));
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
		Client.shutdown();//shutdown internal server
	}
	return 0;
}




