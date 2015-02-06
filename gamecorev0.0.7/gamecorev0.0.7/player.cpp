/************************************************************************/
/* Player.cpp
	purpose: to render the character and animate them
	By: Michael Carey
*/
/************************************************************************/

#include <irrbullet.h>
#include "player.h"

using namespace irr;
using namespace irrklang;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


player::player(IrrlichtDevice* device,char* filename,irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, ISoundEngine* engine, 
			   irrBulletWorld* world):
device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world),
xDirection_(0.0f), zDirection_(0.0f),playerHealth_(PLAYER_HEALTH),walkframe_(0)
{
	chat_message_="\0";
	camera_ = device->getSceneManager()->addCameraSceneNode();
	camera_->bindTargetAndRotation(true);
	camera_->setID(ID_IsNotPickable);
	
	//set up the camera clipping planes in order to render the scene faster
	//camera_->setNearValue(10);
	//camera_->setFarValue(800);
	
	mouseCursorX_ = 0.0;
	mouseCursorY_ = 0.0;

	float height = 5.0;
	float width  = 3.0;

	character_ = new IKinematicCharacterController(world_);
	character_->warp(vector3df(41.0f, 73.0f, 23.0f));//set the init player pos on the map

	//dungeon example
	//character_->warp(vector3df(-217.0f, 3.8f, 390.0f));
	//character_->warp(vector3df(-17.0f, 15.8f, -217.0f));

	characterModel_ = device_->getSceneManager()->addAnimatedMeshSceneNode(device_->getSceneManager()->getMesh(filename));
	
	setTexture("characters/playerskin3.jpg");
	characterModel_->setScale(vector3df(3.0, 3.0, 3.0));
	
	characterModel_->setAnimationSpeed(PLAYER_ANIMATION_SPEED);
	
	
	character_->setGravity(PLAYER_EARTH_GRAVITY);

	character_->setJumpForce(PLAYER_JUMP_FORCE);

	character_->setMaxSlope(PI/4);

	playerSteps_ = engine_->play2D("sounds/footsteps-4.wav", true, true); //Player foot steps sounds. Declare them here and start it off as paused
	
	playerSpeed_ = DEFAULT_PLAYER_SPEED;

	//////////////////////////////////////////////////////////////////////////
	characterModel_->setMaterialFlag(video::EMF_NORMALIZE_NORMALS,1);
	characterModel_->setMaterialFlag(video::EMF_LIGHTING,0);
	characterModel_->setID(ID_IsNotPickable);//so we cannot select our model
	characterModel_->addShadowVolumeSceneNode();//make realtime shadows on the character
	//////////////////////////////////////////////////////////////////////////
	
	idle();
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
	if (character_->isOnGround()){
		character_->jump();
		characterModel_->setFrameLoop(103,111);
	}
}

void player::moveCameraControl()
{

	
	characterModel_->setPosition(character_->getWorldTransform().getTranslation());

	vector3df rot(0, camera_->getRotation().Y, 0);
	characterModel_->setRotation(rot);

	vector3df direction(xDirection_, 0.0f, zDirection_);

	irr::core::matrix4 m;

	m.setRotationDegrees(vector3df(0, camera_->getRotation().Y-180.0f, 0));

	m.transformVect(direction);

	character_->setPositionIncrementPerSimulatorStep(direction*0.3f);

	camera_->setTarget(characterModel_->getPosition());

	// Step the simulation with our delta time

	irr::core::position2d<f32> cursorPos = device_->getCursorControl()->getRelativePosition();
	camera_ = device_->getSceneManager()->getActiveCamera();
	irr::core::vector3df cameraPos = camera_->getAbsolutePosition();

	characterModel_->setPosition(character_->getWorldTransform().getTranslation());

	camera_->setPosition(calculateCameraPos());	

	nodeSelector(); //highlights nodes within a certain range of the playera
	
}

vector3df player::calculateCameraPos()
{
	vector3df newCamPos;

	position2d<f32> cursor = device_->getCursorControl()->getRelativePosition();
	
	mouseCursorX_ += (cursor.X - 0.5f) * CAMERA_MOVE_SPEED;
	mouseCursorY_ -= (cursor.Y - 0.5f) * CAMERA_MOVE_SPEED;

	if( mouseCursorY_ < -90 )
		mouseCursorY_ = -89.9f;
	else
		if( mouseCursorY_ > 40 )
			mouseCursorY_ = 39.9f;

	//std::cout << mouseCursorX_ << ", " << mouseCursorY_ << std::endl;

	newCamPos.X = characterModel_->getPosition().X - (cos(mouseCursorX_ * PI / 180.0f) * CAMERA_DISTANCE_BACK);
	newCamPos.Y = characterModel_->getPosition().Y - (sin(mouseCursorY_ * PI / 180.0f) * CAMERA_DISTANCE_BACK);
	newCamPos.Z = characterModel_->getPosition().Z + (sin(mouseCursorX_ * PI / 180.0f) * CAMERA_DISTANCE_BACK);

	return newCamPos;
}

void player::forward()
{	
	characterModel_->setFrameLoop(301,318);
	characterModel_->setAnimationSpeed(4);

	/*if (walkframe_== 0 || walkframe_ == 319 )
	{
		walkframe_ = 301;
	}
	else
	{
		walkframe_++;
	}

	characterModel_->setCurrentFrame(walkframe_);*/
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
	characterModel_->setFrameLoop(206,250);
}

void player::nodeSelector()
{
	//////////////////////////////////////////////////////////////////////////
	//Early Selection Code
	//////////////////////////////////////////////////////////////////////////
	scene::ISceneCollisionManager* collMan = smgr_->getSceneCollisionManager();
	line3d<f32> ray = collMan->getRayFromScreenCoordinates(device_->getCursorControl()->getPosition(), camera_);
	//core::line3d<f32> ray;

	//ray.start = camera_->getPosition();
	//ray.end = ray.start + (camera_->getTarget()- ray.start) * 100000.0f;
	// Tracks the current intersection point with the level or a mesh
	core::vector3df intersection;
	// Used to show with triangle has been hit
	core::triangle3df hitTriangle;

	scene::ISceneNode * selectedSceneNode =
		collMan->getSceneNodeAndCollisionPointFromRay(
		ray,
		intersection, // This will be the position of the collision
		hitTriangle, // This will be the triangle hit in the collision
		IDFlag_IsPickable, // This ensures that only nodes that we have
		// set up to be pick able are considered
		0); // Check the entire scene (this is actually the implicit default)



	if( selectedSceneNode)
	{

		// We can check the flags for the scene node that was hit to see if it should be
		// highlighted. 
		if(selectedSceneNode->getID() == IDFlag_IsPickable)
		{

			driver_->setTransform(video::ETS_WORLD, core::matrix4());
			driver_->draw3DTriangle(hitTriangle, video::SColor(100,255,0,0));
			//highlightedSceneNode = selectedSceneNode;

			// Highlighting in this case means turning lighting OFF for this node,
			// which means that it will be drawn with full brightness.
			//highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
			//selectedSceneNode->setDebugDataVisible(EDS_MESH_WIRE_OVERLAY);
			//selectedSceneNode->setDebugDataVisible(EDS_OFF);
		}
	}
}
