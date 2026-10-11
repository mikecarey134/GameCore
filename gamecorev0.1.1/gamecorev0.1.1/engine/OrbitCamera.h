#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H

#include <position2d.h>
#include <vector3d.h>

namespace irr { namespace scene { class ISceneManager; class ICameraSceneNode; } }

class OrbitCamera
{
public:
	explicit OrbitCamera(irr::scene::ISceneManager* smgr = 0);

	void update(const irr::core::position2df& mouseDelta);
	void setTarget(const irr::core::vector3df& target) { target_ = target; }
	void zoom(float delta);
	void reset() { yaw_ = 0.0f; pitch_ = 0.0f; }
	void setSensitivity(float degreesPerWindow) { sensitivity_ = degreesPerWindow; }

	float getYaw() const { return yaw_; }
	float getPitch() const { return pitch_; }
	float getDistance() const { return distance_; }
	irr::core::vector3df position() const;
	irr::core::vector3df lookDirection() const;
	irr::scene::ICameraSceneNode* getNode() const { return node_; }
	void makeActive();
	float getMoveYaw() const { return (float)lookDirection().getHorizontalAngle().Y; }

private:
	float yaw_;
	float pitch_;
	float distance_;
	float sensitivity_;
	irr::core::vector3df target_;
	irr::scene::ISceneManager* smgr_;
	irr::scene::ICameraSceneNode* node_;
};

#endif
