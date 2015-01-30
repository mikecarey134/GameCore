/************************************************************************/
/*
	Main.cpp
	Purpose: The main game-loop for "The Uninvited" V0.0.5
	By:Michael Carey && Dan Brown

*/
/************************************************************************/

#define _IRR WINDOWS_

//#define CONSOLE//debugging console
#include "exampleframework.h"
#include <irrbullet.h>
#include <irrlicht.h>
#include <irrKlang.h>
#include <iostream>
//#include "chatclient.h"
#include <Windows.h>
#include "driverChoice.h"
#include "SAppcontext.h"
#include "keymap.h"
#include "Splash.h"
#include "console.h"
#include "consoleevent.h"
#include "player.h"



//standard irrlicht includes
using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


bool OnEvent(irr::SEvent event, IC_Console& console);

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
//#pragma comment(lib, "irrKlang.lib")
#ifdef CONSOLE
#else
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")//comment out if you want to see the Debug console
#pragma comment(lib,"IrrConsole-DEBUG")
#endif
#endif


//*will split the main() into classes later to smooth things out
int main(void)
{

	IrrlichtDevice *device = createDevice(EDT_OPENGL,//standard open_gl drivers
		dimension2d<u32>(800,600),16,false,true,true,0);

	ISoundEngine* engine = createIrrKlangDevice();//add sounds to irrlicht

	//set up the bullet physics here for our world
	irrBulletWorld* world;//the irrbullet world 
	world = createIrrBulletWorld(device, true, true);
	world->setDebugMode(EPDM_DrawAabb | EPDM_DrawContactPoints);
	world->setGravity(vector3df(0,-20,0));
	//device->getSceneManager()->addLightSceneNode(0, vector3df(125, 25, -72), SColorf(255.0f, 255.0f, 255.0f), 4000.0f);

	if(!device)//if device fails to load exit
	{
		return 1;
	}
	if (!engine)//if the irrklang device fails exit 
	{
		return 1;
	}
	
	device->setWindowCaption(L"gamecore v0.0.7");//set the caption for the window
	//device->setResizable(true);
	
	//setup the rest of the irrlicht devices here
	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	keymap userkeys;
	GUI theGui;

	SAppContext context;//guiEvent stuff
	context.device = device;
	context.counter = 0;
	
	player thePlayer(device,"characters/stick.md2",smgr,driver, engine, world);

	device->getSceneManager()->loadScene("map/subbsalpha1.irr");//debug map scene

	//Open World Example///////////////////////////////////////////////////////
	//device->getSceneManager()->loadScene("map/openworld.irr");//debug map scene
	//driver->setFog(SColor(0,255,255,255),EFT_FOG_EXP2,200,400,.0009,false,false);
	////////////////////////////////////////////////////////////////////////////
	
	//thePlayer.animate(device);
		
	consoleevent crecv(device,guienv,driver,context,theGui, engine, &thePlayer, world);//event handeler
	device->setEventReceiver(&crecv);

	gui::IGUIFont* font2 = device->getGUIEnvironment()->getFont("bill/bigfont.png");

	

	core::array<scene::ISceneNode*> nodes;
	smgr->getSceneNodesFromType(scene::ESNT_ANY,nodes);
	
	for (u32 i=0; i < nodes.size(); ++i)
	{
		scene::ISceneNode* node = nodes[i];
		stringc name = node->getName();
		const stringc prefix = name.subString(0,name.findFirst('_'));
		const stringc suffix = name.subString(name.findFirst('_')+1, name.size()-name.findFirst('_'));

		if(node->getType() == scene::ESNT_MESH || scene::ESNT_CUBE || scene::ESNT_SPHERE)
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
				node->setAutomaticCulling(EAC_FRUSTUM_BOX);
				//node->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
				//driver->addOcclusionQuery(node, ((scene::IMeshSceneNode*)node)->getMesh());
			}

			else if(prefix == "static")
			{
				IBvhTriangleMeshShape* shape = new IBvhTriangleMeshShape(node, static_cast<IMeshSceneNode*>(node)->getMesh(), 0.0f);
				IRigidBody* body = world->addRigidBody(shape);
				//node->setMaterialFlag(EMF_FOG_ENABLE,true);//add fog to our scene
				node->setAutomaticCulling(EAC_FRUSTUM_BOX);
				//node->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
				//driver->addOcclusionQuery(node, ((scene::IMeshSceneNode*)node)->getMesh());
				//node->
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
			//node->setMaterialFlag(EMF_LIGHTING, true);
			//static_cast<IMesh*>(node)->setHardwareMappingHint(EHM_STREAM);			
		}
	}


	u32 then = device->getTimer()->getTime();
	bool nodeVisible = true;

	ISound* intro = engine->play2D("sounds/slowintro.mp3", true);

	while(device->run())//while the game is running
	{	/*
		if (device->getTimer()->getTime()-then > 100)	//Loop for manual culling
		{
			for (u32 i=0; i < nodes.size(); ++i)
			{
				scene::ISceneNode* node = nodes[i];
				driver->runAllOcclusionQueries(false);
				driver->updateAllOcclusionQueries();
				nodeVisible = driver->getOcclusionQueryResult(node) > 0;
			}

		}
		*/
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
					if (engine->isCurrentlyPlaying("sounds/slowintro.mp3"))//if we are starting
							engine->removeSoundSource("sounds/slowintro.mp3");//remove the main menu sounds

					driver->beginScene(true,true,video::SColor(0,0,0,0));//begin scene with a white background
					//vector3df nodepos = thePlayer.getPosition();

					//make cursor temp visible until a better menu is in place
						if (!crecv.getPaused() && !crecv.getIsInventory())
						{
							device->getCursorControl()->setVisible(false);
							device->getCursorControl()->setPosition( 0.5f, 0.5f );
						}
						else
							device->getCursorControl()->setVisible(true);

						smgr->drawAll();
						//device->yield();

						world->stepSimulation(frameDeltaTime*0.001f, 120);//bullet psysics
						crecv.update(then,now);
						thePlayer.moveCameraControl();
						
					
						//GUI CLASS calls
						if(font2)
						{
							gui::IGUIFont* font = device->getGUIEnvironment()->getBuiltInFont();
							//draw the current gui related items
							//theGui.drawHealth(font2,core::rect<s32>(10,565,450,100));
							//theGui.drawCrosshair(font2,core::rect<s32>(400,300,450,100));
						}

						//world->debugDrawWorld(true);						
			
					crecv.displayconsole(frameDeltaTime);
					
				}
				
			
			driver->endScene();

		}
		else
			device->yield();
	}

	//reclaim memory and shutdown device
	device->drop();

	return 0;
}

