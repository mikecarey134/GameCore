#ifndef MESSAGEFRAME_H
#define MESSAGEFRAME_H

#include <cstddef>
#include <cstdint>
#include <vector>

const size_t FRAME_HEADER_SIZE = 3;
const size_t MAX_FRAME_PAYLOAD = 0xFFFF;

struct Frame
{
	uint8_t type;
	const uint8_t* payload;
	size_t size;
};

bool appendFrame(std::vector<uint8_t>& packet, uint8_t type, const std::vector<uint8_t>& payload);
std::vector<uint8_t> frameMessage(uint8_t type, const std::vector<uint8_t>& payload);
bool unframe(const uint8_t* data, size_t size, std::vector<Frame>& frames);
bool unframe(const std::vector<uint8_t>& packet, std::vector<Frame>& frames);

#endif
