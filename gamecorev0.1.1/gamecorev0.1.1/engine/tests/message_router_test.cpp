#include "doctest.h"
#include "net/MessageRouter.h"
#include "net/MessageFrame.h"

namespace
{
	struct Log
	{
		std::vector<std::string> lines;
		MessageRouter::Logger logger() { return [this](const std::string& line) { lines.push_back(line); }; }
	};

	std::vector<uint8_t> packetOf(std::initializer_list<std::pair<uint8_t, std::vector<uint8_t> > > messages)
	{
		std::vector<uint8_t> packet;
		for (const auto& message : messages)
			appendFrame(packet, message.first, message.second);
		return packet;
	}
}

TEST_CASE("a handler gets the sending peer and a reader over its payload")
{
	Log log;
	MessageRouter router(log.logger());
	PeerId seenPeer = NO_PEER;
	uint16_t seenValue = 0;
	bool seenBad = true;
	router.on(20, [&](PeerId peer, ByteReader& reader) {
		seenPeer = peer;
		seenValue = reader.readU16();
		seenBad = reader.bad();
	});
	router.route(7, frameMessage(20, { 0x34, 0x12 }));
	CHECK(seenPeer == 7);
	CHECK(seenValue == 0x1234);
	CHECK_FALSE(seenBad);
	CHECK(log.lines.empty());
}

TEST_CASE("several messages in one packet reach their handlers in order")
{
	Log log;
	MessageRouter router(log.logger());
	std::vector<int> order;
	router.on(1, [&](PeerId, ByteReader& reader) { order.push_back(100 + reader.readU8()); });
	router.on(2, [&](PeerId, ByteReader& reader) { order.push_back(200 + reader.readU8()); });
	router.route(1, packetOf({ { 2, { 1 } }, { 1, { 2 } }, { 2, { 3 } } }));
	CHECK(order == std::vector<int>{ 201, 102, 203 });
}

TEST_CASE("a handler's reader ends at its own payload")
{
	Log log;
	MessageRouter router(log.logger());
	bool firstBad = false;
	uint8_t second = 0;
	router.on(1, [&](PeerId, ByteReader& reader) { reader.readU16(); firstBad = reader.bad(); });
	router.on(2, [&](PeerId, ByteReader& reader) { second = reader.readU8(); });
	router.route(1, packetOf({ { 1, { 5 } }, { 2, { 9 } } }));
	CHECK(firstBad);
	CHECK(second == 9);
}

TEST_CASE("an unknown type is dropped and logged once, other messages still route")
{
	Log log;
	MessageRouter router(log.logger());
	int known = 0;
	router.on(1, [&](PeerId, ByteReader& reader) { reader.readU8(); ++known; });
	router.route(3, packetOf({ { 99, { 1 } }, { 1, { 1 } } }));
	router.route(3, packetOf({ { 99, { 2 } } }));
	CHECK(known == 1);
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("99") != std::string::npos);
}

TEST_CASE("each unknown type gets its own log line")
{
	Log log;
	MessageRouter router(log.logger());
	router.route(1, packetOf({ { 60, {} }, { 61, {} }, { 60, {} } }));
	CHECK(log.lines.size() == 2);
}

TEST_CASE("a malformed packet calls no handler and is logged once")
{
	Log log;
	MessageRouter router(log.logger());
	int calls = 0;
	router.on(1, [&](PeerId, ByteReader& reader) { reader.readU8(); ++calls; });
	std::vector<uint8_t> packet = frameMessage(1, { 1 });
	packet.push_back(1);
	router.route(4, packet);
	router.route(4, packet);
	CHECK(calls == 0);
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("4") != std::string::npos);
}

TEST_CASE("registering a type again replaces its handler")
{
	Log log;
	MessageRouter router(log.logger());
	int first = 0, second = 0;
	router.on(1, [&](PeerId, ByteReader&) { ++first; });
	router.on(1, [&](PeerId, ByteReader&) { ++second; });
	router.route(1, frameMessage(1, {}));
	CHECK(first == 0);
	CHECK(second == 1);
}
