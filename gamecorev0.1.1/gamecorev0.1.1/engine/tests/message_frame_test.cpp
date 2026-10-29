#include "doctest.h"
#include "net/MessageFrame.h"

namespace
{
	std::vector<uint8_t> payloadOf(const Frame& frame)
	{
		return std::vector<uint8_t>(frame.payload, frame.payload + frame.size);
	}
}

TEST_CASE("a frame is the type, a little-endian u16 length, then the payload")
{
	CHECK(frameMessage(7, { 1, 2, 3 }) == std::vector<uint8_t>{ 7, 3, 0, 1, 2, 3 });
}

TEST_CASE("a framed message unframes to the same type and payload")
{
	std::vector<uint8_t> packet = frameMessage(42, { 9, 8, 7, 6 });
	std::vector<Frame> frames;
	REQUIRE(unframe(packet, frames));
	REQUIRE(frames.size() == 1);
	CHECK(frames[0].type == 42);
	CHECK(payloadOf(frames[0]) == std::vector<uint8_t>{ 9, 8, 7, 6 });
}

TEST_CASE("several messages in one packet unframe in order")
{
	std::vector<uint8_t> packet;
	CHECK(appendFrame(packet, 1, { 10 }));
	CHECK(appendFrame(packet, 2, {}));
	CHECK(appendFrame(packet, 3, { 30, 31 }));
	std::vector<Frame> frames;
	REQUIRE(unframe(packet, frames));
	REQUIRE(frames.size() == 3);
	CHECK(frames[0].type == 1);
	CHECK(payloadOf(frames[0]) == std::vector<uint8_t>{ 10 });
	CHECK(frames[1].type == 2);
	CHECK(frames[1].size == 0);
	CHECK(frames[2].type == 3);
	CHECK(payloadOf(frames[2]) == std::vector<uint8_t>{ 30, 31 });
}

TEST_CASE("an empty payload is a valid message")
{
	std::vector<uint8_t> packet = frameMessage(5, {});
	CHECK(packet == std::vector<uint8_t>{ 5, 0, 0 });
	std::vector<Frame> frames;
	REQUIRE(unframe(packet, frames));
	REQUIRE(frames.size() == 1);
	CHECK(frames[0].size == 0);
}

TEST_CASE("an empty packet holds no messages")
{
	std::vector<Frame> frames;
	CHECK(unframe(std::vector<uint8_t>(), frames));
	CHECK(frames.empty());
}

TEST_CASE("a truncated header rejects the whole packet")
{
	std::vector<uint8_t> packet = frameMessage(1, { 10 });
	packet.push_back(2);
	packet.push_back(0);
	std::vector<Frame> frames;
	CHECK_FALSE(unframe(packet, frames));
	CHECK(frames.empty());

	std::vector<uint8_t> lone = { 4 };
	CHECK_FALSE(unframe(lone, frames));
}

TEST_CASE("a length past the end of the packet rejects the whole packet")
{
	std::vector<uint8_t> packet = frameMessage(1, { 10 });
	std::vector<uint8_t> overlong = { 2, 3, 0, 20, 21 };
	packet.insert(packet.end(), overlong.begin(), overlong.end());
	std::vector<Frame> frames;
	CHECK_FALSE(unframe(packet, frames));
	CHECK(frames.empty());

	std::vector<uint8_t> exact = { 2, 2, 0, 20, 21 };
	CHECK(unframe(exact, frames));
	CHECK(frames.size() == 1);
}

TEST_CASE("a payload of the maximum size frames, one byte more does not")
{
	std::vector<uint8_t> packet;
	CHECK(appendFrame(packet, 1, std::vector<uint8_t>(MAX_FRAME_PAYLOAD, 0xAA)));
	CHECK(packet.size() == FRAME_HEADER_SIZE + MAX_FRAME_PAYLOAD);
	std::vector<Frame> frames;
	REQUIRE(unframe(packet, frames));
	CHECK(frames[0].size == MAX_FRAME_PAYLOAD);

	std::vector<uint8_t> before = packet;
	CHECK_FALSE(appendFrame(packet, 2, std::vector<uint8_t>(MAX_FRAME_PAYLOAD + 1, 0xBB)));
	CHECK(packet == before);
	CHECK(frameMessage(2, std::vector<uint8_t>(MAX_FRAME_PAYLOAD + 1, 0xBB)).empty());
}

TEST_CASE("unframing a packet replaces whatever the frame list held")
{
	std::vector<Frame> frames;
	unframe(frameMessage(1, { 1 }), frames);
	std::vector<uint8_t> second = frameMessage(2, { 2 });
	REQUIRE(unframe(second, frames));
	CHECK(frames.size() == 1);
	CHECK(frames[0].type == 2);
}
