/************************************************************************/
/*
	Main.cpp
	Purpose: The main game-loop for GameCore v0.1.1
	By:Michael Carey && Dan Brown
*/
/************************************************************************/

#ifdef _WIN32
#define _IRR_WINDOWS_
#endif
//#define CONSOLE
//comment out if you don't want to see the Debug console

#include "ChatQueue.h"
#include "IrrlichtCanvas.h"
#include "physics/PhysicsWorld.h"
#include <irrlicht.h>
#include "SoundEngine.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "common.h"
#include "SAppcontext.h"
#include "console.h"
#include "GameFlow.h"
#include "player.h"
#include "NPC.h"
#include "ParticleSystem.h"
#include "InternalServer.h"
#include "GameMap.h"
#include "MouseCapture.h"
#include "GameLayers.h"
#include "Screens.h"
#include "ConsoleLayer.h"
#include "ActionMap.h"
#include "UiAnchor.h"
#include "defaultCmds.h"
#include "InteractiveObject.h"
#include "ClueObject.h"
#include "Config_Reader.h"	
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "WeaponObject.h"
#include "SolarCycle.h"
#include "Catalog.h"
#include "Inventory.h"
#include "InventoryScreen.h"
#include <fstream>
#include <sstream>


//standard irrlicht namespaces
using namespace irr;
using namespace gamecore;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#if defined(_WIN32) && !defined(_WINDOWS_)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")

#ifdef CONSOLE//if we want the console displayed
#else
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#pragma comment(lib,"IrrConsole-DEBUG")
#endif
#endif


static std::string readText(const char* path)
{
	std::ifstream in(path, std::ios::binary);
	std::ostringstream text;
	text << in.rdbuf();
	return text.str();
}

static void addStartingItems(inventory::Inventory& backpack)
{
	const char* items[] = { "pistol", "knife", "shotgun", "ammo_box", "key", "lockpick", "pills", "body_armor", "utility_belt" };
	for (const char* item : items)
		backpack.pickUp(inventory::Item{ item, 1 });
	backpack.pickUp(inventory::Item{ "bandage", 3 });

	const char* worn[][2] = { { "body_armor", "chest" }, { "utility_belt", "belt" } };
	for (const auto& pair : worn)
		for (const inventory::Placed& placed : backpack.backpackContents())
			if (placed.item.type == pair[0])
			{
				backpack.equipFrom(placed.anchor, pair[1]);
				break;
			}
}

int main(int argc, char** argv)
{
	const bool smoke = argc > 1 && std::string(argv[1]) == "--smoke";
	//seed the random time
	srand((unsigned int)time(NULL));
	//read in our config file
	Config_Reader config_reader("config/configure.txt");
	
	//setup the irrlicht devices here
	IrrlichtDevice *device = createDevice(smoke ? EDT_NULL : config_reader.get_driver(),//drivers
	config_reader.get_window_size(),16,config_reader.get_full_screen()
	,config_reader.get_s_buffer(),config_reader.get_vsinc());
	if(!device)//if device fails to load exit
	{
		return 1;
	}

	IrrlichtCanvas canvas(device);

	drawIntro(device);//draw our intro 


	
	device->setWindowCaption(L"gamecore Build v0.1.1(Alpha)");//set the caption for the window
	SoundEngine* engine = createSoundEngine();//add sounds to irrlicht
	if (!engine)//if the sound engine fails exit 
	{
		device->drop();
		return 1;
	}
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	IVideoDriver* driver = device->getVideoDriver();

	//load our scene manager and set up the shadow color with an alpha channel
	ISceneManager* smgr = device->getSceneManager();smgr->setShadowColor(irr::video::SColor(70,0,0,0));
	//keymap userkeys;

	PhysicsWorld* world = PhysicsWorld::create(device);
	world->setGravity(-20.0f);

	SAppContext context;//guiEvent stuff
	context.device = device;
	context.counter = 0;

	//init the chat buffer display that shows all player chat
	ChatQueue chat_queue(device,canvas,anchored(rect<s32>(0,420,580,500), 0.0f, 1.0f, canvas.screenSize()),5000,true,500,true,false);
	//set up the chat buffers font
	chat_queue.setFont(canvas.loadFont("bill/console.bmp"));

	drawLoading(device);//load gui and display loading while the computer is working
	

	//Loading map now handled in its own class
	GameMap currentMap(device,driver,smgr,world);
	currentMap.setMap(PREFIX "mountains.irr");
	//currentMap.setMap("nm.irr");
	//WeaponObject testWeap("characters/knife/knife.obj", device,vector3df(132,37,-57));
		//takes care of all input devices and events calls update during the gameloop

	//load our player into the scene
	ActionMap actions;
	bindDefaultActions(actions);
	actions.loadFile("config/keybinds.txt", [](const std::string& message) { std::cerr << message << std::endl; });

	player thePlayer(device,actions,"characters/stick_mike.ms3d",smgr,driver, engine, world,config_reader.get_player_name()
		,config_reader.get_model());

	SolarCycle sun(device,vector3df(thePlayer.getPosition().X,thePlayer.getPosition().Y+400,thePlayer.getPosition().Z));

	LuaVM luaVM;
	NPC npc_tester(device,smgr,world,driver,luaVM);

	//init the client side of the server
	InternalServer Client(config_reader.get_online() && !smoke,device,"characters/stick_mike.ms3d",smgr,driver,engine,world,config_reader.get_ip()
		,config_reader.get_rem_port(), config_reader.get_port(),&chat_queue, &thePlayer);

	GameFlow gameFlow(device,canvas,guienv,driver,context, engine, &thePlayer, world, &npc_tester, &currentMap, &Client);
	luaVM.setLogger([&gameFlow](const std::string& message) {
		gameFlow.getConsole().logMessage_ANSI(ELL_ERROR, message.c_str());
		std::cerr << message << std::endl;
	});
	gameFlow.getConsole().registerCommand(new IC_Command_LUA(&luaVM));
	InputRouter inputRouter;
	GameplayLayer gameplayLayer(gameFlow, actions, inputRouter.state());
	inputRouter.push(&gameplayLayer);
	DebugLayer debugLayer(gameFlow, actions);
	inputRouter.push(&debugLayer);
	MenuLayer menuLayer(gameFlow, actions);
	inputRouter.push(&menuLayer);
	InventoryLayer inventoryLayer(gameFlow, actions, engine);
	inputRouter.push(&inventoryLayer);
	NoteLayer noteLayer(gameFlow);
	inputRouter.push(&noteLayer);
	ConsoleLayer consoleLayer(gameFlow.getConsole(), actions, [&gameFlow]() { return !gameFlow.getPaused(); });
	inputRouter.push(&consoleLayer);
	device->setEventReceiver(&inputRouter);
	MouseCapture mouseCapture(device->getCursorControl());

	inventory::Catalog itemCatalog;
	inventory::LoadError itemError;
	bool itemsLoaded = inventory::loadCatalog(readText("inventory/data/slots.csv"),
		readText("inventory/data/items.csv"), itemCatalog, itemError);
	inventory::Inventory backpack(itemCatalog, 8, 5);
	std::string itemProblem = itemError.file + " line " + std::to_string(itemError.line) + ": " + itemError.message;
	if (itemsLoaded)
		addStartingItems(backpack);
	else
		std::cerr << "Inventory data did not load: " << itemProblem << std::endl;
	InventoryScreen inventoryScreen = itemsLoaded
		? InventoryScreen(device, backpack, itemCatalog)
		: InventoryScreen(device, itemProblem);
	inventoryScreen.setCloseKeyName(actions.keyName("inventory"));
	gameFlow.setInventoryScreen(&inventoryScreen);

	device->getGUIEnvironment()->getFont("bill/bigfont.png");

	currentMap.loadMap();
	std::vector<vector3df> spawns = currentMap.getSpawnPoints();
	thePlayer.spawn(spawns[rand() % spawns.size()]);
	//thePlayer.setUpCameraCollisions();

	ClueObject testClue("models/clue/clue2.obj",device,smgr,driver,&currentMap,luaVM);

	//player.addClue(testClue);
	gameFlow.addClueObject(&testClue);
	currentMap.setClue(&testClue);
	currentMap.setNpc(&npc_tester);

	u32 then = device->getTimer()->getTime();

	engine->play2D("sounds/discovery.mp3", true);	
	Sound* gameMusic = 0;

	
	//clear the console before the game starts
	gameFlow.clear_console();
	
	//thePlayer.switch_weap(1);
	if (smoke)
		gameFlow.startPressed();
	while(device->run())//while the game is running
	{		
		const u32 now = device->getTimer()->getTime();
		const u32 frameDeltaTime = now - then;	// Time in milliseconds
		then = now;	

		Client.messageLoop(thePlayer,guienv->getBuiltInFont());

		if (smoke || device->isWindowActive())//if a window is running
		{	
				mouseCapture.setCaptured(gameFlow.getStarted() && !gameFlow.getPaused() && !gameFlow.getIsInventory());
				mouseCapture.beginFrame();
				
				if (!gameFlow.getStarted())
				{
					if (!engine->isCurrentlyPlaying("sounds/discovery.mp3"))
						engine->play2D("sounds/discovery.mp3", true);//startup the main menu track
				}
				else
				{
					if (!gameMusic || gameMusic->isFinished())
					{
						if (gameMusic)
							gameMusic->drop();
						gameMusic = engine->play2D("sounds/sunshine.mp3", true, false, true);//startup the main track
					}
					if (gameMusic->getIsPaused() != gameFlow.getPaused())
						gameMusic->setIsPaused(gameFlow.getPaused());
					if (engine->isCurrentlyPlaying("sounds/discovery.mp3"))//if we are starting
						engine->removeSoundSource("sounds/discovery.mp3");//remove the main menu sounds

				}

				driver->beginScene(true,true,video::SColor(255,0,0,0));

				if (gameFlow.getStarted())
				{
					smgr->drawAll();
					world->step(frameDeltaTime*0.001f);
					gameFlow.update(then,now);//update our game here

					//we dont want the camera to update while the player is stationary
					if (!gameFlow.getPaused() && !gameFlow.getIsInventory()&& !gameFlow.getIsConsole())
					{
						thePlayer.moveCameraControl(mouseCapture.delta());
						testClue.update();

						if(!npc_tester.isDead() && npc_tester.isLoaded() == DONE_LOADING)
							npc_tester.moveNPC();//move our npc 
					}
					else
						thePlayer.haltCamera();
				}

				gameFlow.draw();
				if (gameFlow.getStarted())
				{
					chat_queue.draw();
					gameFlow.displayconsole(frameDeltaTime);
				}

			driver->endScene();

		}
		else
			device->yield();

		if (smoke)
		{
			std::cout << "Smoke run finished one frame" << std::endl;
			break;
		}
	}

	if(Client.isOnline())//if we are online
	{
		Client.shutdown(thePlayer);//shutdown internal server
	}

	//reclaim memory and shutdown device
	delete world;
	engine->drop();
	device->drop();

	return 0;
}




