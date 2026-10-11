#include "MessageFrame.h"

bool appendFrame(std::vector<uint8_t>& packet, uint8_t type, const std::vector<uint8_t>& payload)
{
	if (payload.size() > MAX_FRAME_PAYLOAD)
		return false;
	packet.push_back(type);
	packet.push_back((uint8_t)(payload.size() & 0xFF));
	packet.push_back((uint8_t)(payload.size() >> 8));
	packet.insert(packet.end(), payload.begin(), payload.end());
	return true;
}

std::vector<uint8_t> frameMessage(uint8_t type, const std::vector<uint8_t>& payload)
{
	std::vector<uint8_t> packet;
	appendFrame(packet, type, payload);
	return packet;
}

bool unframe(const uint8_t* data, size_t size, std::vector<Frame>& frames)
{
	frames.clear();
	size_t position = 0;
	while (position < size)
	{
		if (size - position < FRAME_HEADER_SIZE)
		{
			frames.clear();
			return false;
		}
		uint8_t type = data[position];
		size_t length = (size_t)data[position + 1] | ((size_t)data[position + 2] << 8);
		position += FRAME_HEADER_SIZE;
		if (length > size - position)
		{
			frames.clear();
			return false;
		}
		frames.push_back(Frame{ type, data + position, length });
		position += length;
	}
	return true;
}

bool unframe(const std::vector<uint8_t>& packet, std::vector<Frame>& frames)
{
	return unframe(packet.data(), packet.size(), frames);
}
