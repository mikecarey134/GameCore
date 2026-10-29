#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <cstddef>
#include <cstdint>
#include <deque>

struct Pose
{
	float position[3];
	float yaw;
};

bool isNewerSeq(uint16_t seq, uint16_t than);

class Interpolator
{
public:
	static const size_t CAPACITY = 3;
	static const float SNAP_DISTANCE;
	static const float DELAY;

	void addSnapshot(uint16_t seq, const float position[3], float yaw, float receiveTime);
	bool sample(float renderTime, Pose& pose) const;
	void reset();
	size_t size() const { return snapshots_.size(); }

private:
	struct Snapshot
	{
		uint16_t seq;
		Pose pose;
		float time;
	};

	std::deque<Snapshot> snapshots_;
};

#endif
