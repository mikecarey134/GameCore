#include "MessageRouter.h"
#include "MessageFrame.h"

MessageRouter::MessageRouter(Logger log) : log_(log), loggedMalformed_(false)
{
	for (int i = 0; i < 256; ++i)
		loggedUnknown_[i] = false;
}

void MessageRouter::on(uint8_t type, Handler handler)
{
	handlers_[type] = handler;
}

void MessageRouter::route(PeerId peer, const std::vector<uint8_t>& packet)
{
	std::vector<Frame> frames;
	if (!unframe(packet, frames))
	{
		if (!loggedMalformed_)
			log_("Dropped a malformed packet from peer " + std::to_string(peer));
		loggedMalformed_ = true;
		return;
	}

	for (size_t i = 0; i < frames.size(); ++i)
	{
		uint8_t type = frames[i].type;
		if (!handlers_[type])
		{
			if (!loggedUnknown_[type])
				log_("Dropped a message of unknown type " + std::to_string(type) + " from peer " + std::to_string(peer));
			loggedUnknown_[type] = true;
			continue;
		}
		ByteReader reader(frames[i].payload, frames[i].size);
		handlers_[type](peer, reader);
	}
}
