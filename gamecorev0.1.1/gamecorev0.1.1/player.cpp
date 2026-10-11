/************************************************************************/
/* Player.cpp
	purpose: to render the character and animate them
	By: Michael Carey
*/
/************************************************************************/

#include "player.h"
#include <fstream>

using namespace irr;
using namespace gamecore;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


player::player(IrrlichtDevice* device,const ActionMap& actions,const char*,irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, SoundEngine* engine, 
			   PhysicsWorld* physics, std::string player_name,int model_type):
camera_(smgr),physics_(physics),device_(device),smgr_(smgr), driver_(driver), engine_(engine),
xDirection_(0.0f), zDirection_(0.0f),playerHealth_(PLAYER_HEALTH),playerHunger_(PLAYER_HUNGER),playerWater_(PLAYER_WATER),walkframe_(0), player_name_(player_name),
player_model_type_(model_type),gore_(smgr_,driver_,irr::core::aabbox3df(-10,48,-10,10,30,10),vector3df(0.0f,0.3f,0.0f),300,100
									,SColor(150,203,104,86),SColor(150,167,90,65),800,200,90,dimension2df(0.2f,0.2f),dimension2df(4.0,4.0))
{
	//append number to players to make 
	//sure the same player id is not created
	player_name_ += "_";
	for (int i=0;i<4;++i)
	{
		int id = (rand() % 10) +48;
		player_name_+= (char)id;
	}
	
	//set the init chat message to an empty string
	chat_message_="\0";

	//key mapping for the observer camera
	const char* moves[] = { "move_forward", "move_back", "move_left", "move_right" };
	const EKEY_ACTION cameraMoves[] = { EKA_MOVE_FORWARD, EKA_MOVE_BACKWARD, EKA_STRAFE_LEFT, EKA_STRAFE_RIGHT };
	core::array<SKeyMap> keyMap;
	for (int i = 0; i < 4; ++i)
		for (size_t k = 0; k < actions.keys(moves[i]).size(); ++k)
			keyMap.push_back(SKeyMap(cameraMoves[i], actions.keys(moves[i])[k]));


	//////////////////////////////////////////////////////////////////////////
	//player cameras
	//////////////////////////////////////////////////////////////////////////
	camera_.getNode()->setID(ID_IsNotPickable);

	fpsCam_ = smgr_->addCameraSceneNodeFPS(0,75,0.2F,-1,keyMap.pointer(),keyMap.size());
	fpsCam_->setID(ID_IsNotPickable);
	camera_.makeActive();
	//set up the camera clipping planes in order to render the scene faster

	camera_.getNode()->setNearValue(5);
	camera_.getNode()->setFarValue(1500);
	//////////////////////////////////////////////////////////////////////////
		
	createCharacter();

	//dungeon example
	//character_->warp(vector3df(-217.0f, 3.8f, 390.0f));
	//character_->warp(vector3df(-17.0f, 15.8f, -217.0f));

	//////////////////////////////////////////////////////////////////////////
	//player attributes
	//////////////////////////////////////////////////////////////////////////
	common_paths model_path;

	characterModel_ = smgr_->addAnimatedMeshSceneNode(smgr_->getMesh(model_path.model_paths[player_model_type_].c_str()));

	//setTexture("characters/playerskin3.jpg");
	setTexture(model_path.skin_paths[player_model_type_].c_str());

	characterModel_->setScale(vector3df(1.0, 1.0, 1.0));
	
	characterModel_->setAnimationSpeed(PLAYER_ANIMATION_SPEED);

	//const c8 modelName = player_name_;
	//characterModel_->setName(&modelName);

	//add the player shadow
	player_shadow_ = 0;
	//player_shadow_ = smgr_->addMeshSceneNode(smgr_->getMesh("models/shadows/shadow.ms3d"));
	//player_shadow_->setMaterialTexture(0,driver_->getTexture("models/shadows/shadow1.png"));

	playerSpeed_ = DEFAULT_PLAYER_SPEED;

	playerSteps_ = engine_->play2D("sounds/footsteps-4.wav", true, true); //Player foot steps sounds. Declare them here and start it off as paused
	playerSteps_->setVolume(.55f);
	//////////////////////////////////////////////////////////////////////////	

	//////////////////////////////////////////////////////////////////////////
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,1);
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,0);
	characterModel_->setID(ID_IsNotPickable);//so we cannot select our model
	
	
	//setup the player shadow
	//player_shadow_->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);


	//////////////////////////////////////////////////////////////////////////

	enemyInRange_ = false;
	clueInRange_ = false;
	enemyNPCInRange_ = false;
	enemyPlayerInRange_ = false;
	
	idle();

	lamp_= device_->getSceneManager()->addLightSceneNode(0,characterModel_->getPosition(),
		SColor(255,255,255,255),12);//add our lamp here
	setlamp(false);

	clues_.clear();

	camera_.getNode()->setPosition(characterModel_->getPosition());
	
	//good CPU feature
	characterModel_->addShadowVolumeSceneNode();//make realtime shadows on the character
}

player::~player(void)
{

}
void player::animate(EMD2_ANIMATION_TYPE animation)
{

	
	characterModel_->setMaterialFlag(video::EMF_LIGHTING, true);
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	
	characterModel_->setMD2Animation(animation);
}

void player::jump()
{
	//animate the player on jump
	if (character_->onGround()){
		player_jump_= engine_->play2D("sounds/jump.mp3");
		character_->jump();
		characterModel_->setFrameLoop(103,111);
		current_state_=State::JUMP;
	}
}

//Camera Collision code from irrlicht collisions example
void player::setUpCameraCollisions()
{
	scene::IMetaTriangleSelector * meta = device_->getSceneManager()->createMetaTriangleSelector();

	/*
	Now we will find all the nodes in the scene and create triangle
	selectors for all suitable nodes.  Typically, you would want to make a
	more informed decision about which nodes to performs collision checks
	on; you could capture that information in the node name or Id.
	*/
	core::array<scene::ISceneNode *> nodes;
	device_->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, nodes); // Find all nodes

	for (u32 i=0; i < nodes.size(); ++i)
	{
		scene::ISceneNode * node = nodes[i];
		scene::ITriangleSelector * selector = 0;

		switch(node->getType())
		{
		case scene::ESNT_CUBE:
		case scene::ESNT_ANIMATED_MESH:
			// Because the selector won't animate with the mesh,
			// and is only being used for camera collision, we'll just use an approximate
			// bounding box instead of ((scene::IAnimatedMeshSceneNode*)node)->getMesh(0)
			selector = device_->getSceneManager()->createTriangleSelectorFromBoundingBox(node);
		break;

		case scene::ESNT_MESH:
		case scene::ESNT_SPHERE: // Derived from IMeshSceneNode
			selector = device_->getSceneManager()->createTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node);
			break;

		case scene::ESNT_TERRAIN:
			selector = device_->getSceneManager()->createTerrainTriangleSelector((scene::ITerrainSceneNode*)node);
			break;

		case scene::ESNT_OCTREE:
			selector = device_->getSceneManager()->createOctreeTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node);
			break;

		default:
			// Don't create a selector for this node type
			break;
		}

		if(selector)
		{
			// Add it to the meta selector, which will take a reference to it
			meta->addTriangleSelector(selector);
			// And drop my reference to it, so that the meta selector owns it.
			selector->drop();
		}
	}

	scene::ISceneNodeAnimator* anim = device_->getSceneManager()->createCollisionResponseAnimator(
		meta, camera_.getNode(), core::vector3df(1,1,1),
		core::vector3df(0,0,0));
	meta->drop(); // I'm done with the meta selector now

	camera_.getNode()->addAnimator(anim);
	anim->drop(); // I'm done with the animator now
}

void player::moveCameraControl(const irr::core::position2df& mouseDelta)
{
/*
	if (current_state_ == DEAD)
	{
		//////////////////////////////////////////////////////////////////////////
		//early fps camera switching
		//////////////////////////////////////////////////////////////////////////
		if(floor(cameradist_) == 1)
		{
			characterModel_->setPosition(fpsCam_->getPosition());
			characterModel_->setVisible(false);

		}
		//////////////////////////////////////////////////////////////////////////
	}
	else*/
	if (!isDead())
	{
		camera_.makeActive();
		
		characterModel_->setVisible(true);

		characterModel_->setPosition(character_->position());

		vector3df rot(0, camera_.getMoveYaw(), 0);
		characterModel_->setRotation(rot);

		vector3df direction(xDirection_, 0.0f, zDirection_);

		irr::core::matrix4 m;

		m.setRotationDegrees(vector3df(0, camera_.getMoveYaw()-180.0f, 0));

		m.transformVect(direction);

		character_->setWalk(direction*0.3f*60.0f);

		characterModel_->setPosition(character_->position());
		//manage shadow
		//player_shadow_->setPosition(vector3df(getPosition().X,getPosition().Y-3.8f,getPosition().Z));
		//player_shadow_->setScale(vector3df(.35,.35,.35));

		camera_.setTarget(characterModel_->getPosition());
		camera_.update(mouseDelta);

		gore_.set_min_max_amounts(0,0);gore_.setUpEmitter();

		nodeSelector(); //highlights nodes within a certain range of the player	
	}
	else
		fpsCam_->setInputReceiverEnabled(true);

	//setup the player lamp position
	vector3df lamppos;
	lamppos.X = characterModel_->getPosition().X + camera_.lookDirection().X * 4;
	lamppos.Y = characterModel_->getPosition().Y ;
	lamppos.Z = characterModel_->getPosition().Z + camera_.lookDirection().Z * 4;

	//lamp_->enableCastShadow(false);
	
	lamp_->setPosition(lamppos);

	

	
	
}
void player::forward()
{	
	//early walk animations 
	//choppy
	characterModel_->setFrameLoop(301,318);
	characterModel_->setAnimationSpeed(4);
	
	u32 time = device_->getTimer()->getTime();
	
	if((time % 4) == 0)
		walkframe_++;

	if (walkframe_== 0 || walkframe_ >= 319 )
	{
		walkframe_ = 301;
	}
	
	characterModel_->setCurrentFrame((f32)walkframe_);
	current_state_=State::WALK;

	
	



}
void player::backwards()
{	
	
}
void player::left()
{
}
void player::right()
{	
}

void player::idle()
{
	//setup the frame loop for when the player idles
	characterModel_->setAnimationSpeed(4);
	characterModel_->setFrameLoop(206,250);
	current_state_=State::IDLE;
	
}

void player::nodeSelector()
{
	//////////////////////////////////////////////////////////////////////////
	//Early Selection Code
	//////////////////////////////////////////////////////////////////////////
	scene::ISceneCollisionManager* collMan = smgr_->getSceneCollisionManager();
	//line3d<f32> ray = collMan->getRayFromScreenCoordinates(device_->getCursorControl()->getPosition(), camera_);
	core::line3d<f32> ray;

	//ray.start = camera_->getPosition();
	ray.start = vector3df(characterModel_->getPosition().X, 
						  characterModel_->getPosition().Y + 5.0f, 
						  characterModel_->getPosition().Z);
	//ray.end = ray.start + (camera_->getTarget() - ray.start) * 100000.0f;
	//f32 rayEndX = abs(characterModel_->getPosition()) - abs(camera_->getPosition())
	ray.end = characterModel_->getPosition() + camera_.lookDirection() * REACH_DIST;

	// Tracks the current intersection point with the level or a mesh
	core::vector3df intersection;
	// Used to show with triangle has been hit
	core::triangle3df hitTriangle;

	selectedSceneNode_ =
		collMan->getSceneNodeAndCollisionPointFromRay(
		ray,
		intersection, // This will be the position of the collision
		hitTriangle, // This will be the triangle hit in the collision
		IDFlag_IsPickable, // This ensures that only nodes that we have
		// set up to be pick able are considered
		0); // Check the entire scene (this is actually the implicit default)

	if( selectedSceneNode_)
	{

		// We can check the flags for the scene node that was hit to see if it should be
		// highlighted.
		
		if(selectedSceneNode_->getID() == IDFlag_IsPickable)
		{
			std::string nodeName = selectedSceneNode_->getName();	//Need to convert from c8* into a std string
																	//so it will work with the if statement
			std::cout << "Looking at " << nodeName << std::endl;
			if (nodeName == "test_npc")	
			{
				enemyNPCInRange_ = true;
				enemyPlayerInRange_ = false;
			}
			else if (nodeName.substr(0,6) == TYPE_PLAYER)
			{
				enemyPlayerInRange_ = true;
				enemyNPCInRange_ = false;
			}
			else
			{
				enemyNPCInRange_ = false;
				enemyPlayerInRange_ = false;
			}

			if(nodeName.substr(0,4) == TYPE_CLUE)
			{
				clueInRange_ = true;
				//selectedSceneNode_->setDebugDataVisible(EDS_MESH_WIRE_OVERLAY);
				//selectedSceneNode_->setDebugDataVisible(EDS_OFF);
			}
			else
			{
				clueInRange_ = false;
			}
		
			driver_->setTransform(video::ETS_WORLD, core::matrix4());
			//driver_->draw3DTriangle(hitTriangle, video::SColor(100,255,0,0));
			//highlightedSceneNode = selectedSceneNode;
			// Highlighting in this case means turning lighting OFF for this node,
			// which means that it will be drawn with full brightness.
			//highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
			//selectedSceneNode_->setDebugDataVisible(EDS_MESH_WIRE_OVERLAY);
			
		}
		else
		{
			enemyInRange_ = false;
			clueInRange_ = false;
			enemyNPCInRange_ = false;
			enemyPlayerInRange_ = false;			
		}		
	}
	else
	{
		enemyInRange_ = false;
		clueInRange_ = false;
		enemyNPCInRange_ = false;
		enemyPlayerInRange_ = false;
	}
}

void player::addClue(ClueObject* clue)
{
	if (clue->getObjectType() == TYPE_CLUE)
	{
		clues_.push_back(clue);
	}
}
void player::attack()
{
	characterModel_->setFrameLoop(32,44);
	characterModel_->setAnimationSpeed(15);
	player_punch_= engine_->play2D("sounds/swing.mp3");
	current_state_= State::ATTACK;
}

void player::isAttackingSomeone(bool& attacking, std::string& enemyName)
{
	attacking = current_state_ == ATTACK;
	enemyName = selectedSceneNode_->getName();
}

void player::damage()
{
	if (isDead())
		return;
	
	player_punch_= engine_->play2D("sounds/damage.mp3");
	gore_.setPosition(vector3df(getPosition().X,getPosition().Y-40,getPosition().Z));
	gore_.setUpMaterials("textures/blood.png");
	gore_.set_min_max_amounts(100,1000);
	gore_.setUpEmitter();

	playerHealth_ -= 5;

	if (playerHealth_ <= 0)
	{
		current_state_ = DEAD;
		kill();
	}
	
	
}

void player::kill()
{	
	//remove all player model attributes
	gore_.hide(true);
	characterModel_->setVisible(false); 
	if (player_shadow_)
		player_shadow_->setVisible(false);
	delete character_;
	character_ = 0;
	smgr_->setActiveCamera(fpsCam_);
}

void player::createCharacter()
{
	CharacterSettings settings;
	settings.gravity = PLAYER_EARTH_GRAVITY;
	settings.jumpSpeed = PLAYER_JUMP_FORCE;
	settings.maxSlope = (f32)(PI / 1.8);
	character_ = physics_->addCharacter(settings);
}

void player::respawn(vector3df pos)
{
	if (!character_)
		createCharacter();
	playerHealth_ = PLAYER_HEALTH;
	playerHunger_ = PLAYER_HUNGER;
	playerWater_ = PLAYER_WATER;
	camera_.reset();
	clues_.clear();
	gore_.hide(false);
	characterModel_->setVisible(true);
	camera_.makeActive();
	idle();
	spawn(pos);
}

//early player weapon switching system
void player::spawn(vector3df pos)
{
	fpsCam_->setPosition(pos);
	if (character_)
		character_->warp(pos);
	characterModel_->setPosition(pos);
}

void player::switch_weap(int type)
{
	common_paths model_path;
	const char* meshes[] = { model_path.model_paths[player_model_type_].c_str(), "characters/stick_mike_knife.ms3d" };
	if (type < 0 || type > 1)
		return;

	s32 start = characterModel_->getStartFrame();
	s32 end = characterModel_->getEndFrame();
	characterModel_->setMesh(smgr_->getMesh(meshes[type]));
	characterModel_->setFrameLoop(start, end);
	characterModel_->setAnimationSpeed(PLAYER_ANIMATION_SPEED);
	setTexture(model_path.skin_paths[player_model_type_].c_str());
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,1);
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,0);
}