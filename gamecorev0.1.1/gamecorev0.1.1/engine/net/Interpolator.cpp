#include "Interpolator.h"
#include <cmath>

const size_t Interpolator::CAPACITY;
const float Interpolator::SNAP_DISTANCE = 10.0f;
const float Interpolator::DELAY = 0.1f;

bool isNewerSeq(uint16_t seq, uint16_t than)
{
	return (int16_t)(uint16_t)(seq - than) > 0;
}

void Interpolator::addSnapshot(uint16_t seq, const float position[3], float yaw, float receiveTime)
{
	if (!snapshots_.empty() && !isNewerSeq(seq, snapshots_.back().seq))
		return;
	Snapshot snapshot;
	snapshot.seq = seq;
	for (int i = 0; i < 3; ++i)
		snapshot.pose.position[i] = position[i];
	snapshot.pose.yaw = yaw;
	snapshot.time = receiveTime;
	snapshots_.push_back(snapshot);
	if (snapshots_.size() > CAPACITY)
		snapshots_.pop_front();
}

bool Interpolator::sample(float renderTime, Pose& pose) const
{
	if (snapshots_.empty())
		return false;
	if (renderTime <= snapshots_.front().time)
	{
		pose = snapshots_.front().pose;
		return true;
	}
	for (size_t i = 1; i < snapshots_.size(); ++i)
	{
		const Snapshot& from = snapshots_[i - 1];
		const Snapshot& to = snapshots_[i];
		if (renderTime > to.time)
			continue;
		float distanceSquared = 0.0f;
		for (int axis = 0; axis < 3; ++axis)
		{
			float delta = to.pose.position[axis] - from.pose.position[axis];
			distanceSquared += delta * delta;
		}
		if (distanceSquared > SNAP_DISTANCE * SNAP_DISTANCE || to.time <= from.time)
		{
			pose = to.pose;
			return true;
		}
		float t = (renderTime - from.time) / (to.time - from.time);
		for (int axis = 0; axis < 3; ++axis)
			pose.position[axis] = from.pose.position[axis] + (to.pose.position[axis] - from.pose.position[axis]) * t;
		pose.yaw = from.pose.yaw + std::remainder(to.pose.yaw - from.pose.yaw, 360.0f) * t;
		return true;
	}
	pose = snapshots_.back().pose;
	return true;
}

void Interpolator::reset()
{
	snapshots_.clear();
}
