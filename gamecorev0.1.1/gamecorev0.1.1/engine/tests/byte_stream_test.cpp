#include "doctest.h"
#include "net/ByteStream.h"
#include <cmath>

TEST_CASE("each number type is written little-endian and reads back")
{
	ByteWriter writer;
	writer.writeU8(0xAB);
	writer.writeU16(0x1234);
	writer.writeU32(0xDEADBEEF);
	writer.writeF32(-1.5f);
	const std::vector<uint8_t> expected = { 0xAB, 0x34, 0x12, 0xEF, 0xBE, 0xAD, 0xDE, 0x00, 0x00, 0xC0, 0xBF };
	CHECK(writer.data() == expected);

	ByteReader reader(writer.data());
	CHECK(reader.readU8() == 0xAB);
	CHECK(reader.readU16() == 0x1234);
	CHECK(reader.readU32() == 0xDEADBEEF);
	CHECK(reader.readF32() == -1.5f);
	CHECK_FALSE(reader.bad());
}

TEST_CASE("floats keep their exact value through a round trip")
{
	const float values[] = { 0.0f, 123.456f, -98765.4321f, 1e-30f, 3.0e38f };
	ByteWriter writer;
	for (float value : values)
		writer.writeF32(value);
	ByteReader reader(writer.data());
	for (float value : values)
		CHECK(reader.readF32() == value);
	CHECK_FALSE(reader.bad());
}

TEST_CASE("a string is written with a u16 length prefix and reads back")
{
	ByteWriter writer;
	CHECK(writer.writeString("hello", 32));
	const std::vector<uint8_t> expected = { 5, 0, 'h', 'e', 'l', 'l', 'o' };
	CHECK(writer.data() == expected);

	ByteReader reader(writer.data());
	CHECK(reader.readString(32) == "hello");
	CHECK_FALSE(reader.bad());
}

TEST_CASE("an empty string round-trips as a zero length")
{
	ByteWriter writer;
	CHECK(writer.writeString("", 8));
	CHECK(writer.data() == std::vector<uint8_t>{ 0, 0 });
	ByteReader reader(writer.data());
	CHECK(reader.readString(8).empty());
	CHECK_FALSE(reader.bad());
}

TEST_CASE("a string longer than its maximum is not written")
{
	ByteWriter writer;
	CHECK_FALSE(writer.writeString("toolong", 6));
	CHECK(writer.data().empty());
	CHECK(writer.writeString("exactly", 7));
	CHECK(writer.data().size() == 9);
}

TEST_CASE("a string longer than the reader's maximum marks the reader bad")
{
	ByteWriter writer;
	writer.writeString("abcdefgh", 32);
	ByteReader reader(writer.data());
	CHECK(reader.readString(4).empty());
	CHECK(reader.bad());
}

TEST_CASE("a string of exactly the reader's maximum reads, one more does not")
{
	ByteWriter writer;
	writer.writeString("abcde", 32);
	ByteReader exact(writer.data());
	CHECK(exact.readString(5) == "abcde");
	CHECK_FALSE(exact.bad());
	ByteReader over(writer.data());
	CHECK(over.readString(4).empty());
	CHECK(over.bad());
}

TEST_CASE("a string whose length runs past the end marks the reader bad")
{
	const std::vector<uint8_t> data = { 10, 0, 'a', 'b' };
	ByteReader reader(data);
	CHECK(reader.readString(32).empty());
	CHECK(reader.bad());
}

TEST_CASE("reading past the end marks the reader bad and every later read returns zero")
{
	const std::vector<uint8_t> data = { 7 };
	ByteReader reader(data);
	CHECK(reader.readU32() == 0);
	CHECK(reader.bad());
	CHECK(reader.readU8() == 0);
	CHECK(reader.readU16() == 0);
	CHECK(reader.readF32() == 0.0f);
	CHECK(reader.readString(8).empty());
	CHECK(reader.bad());
}

TEST_CASE("an empty reader is fine until something is read from it")
{
	ByteReader reader(nullptr, 0);
	CHECK_FALSE(reader.bad());
	CHECK(reader.readU8() == 0);
	CHECK(reader.bad());
}

TEST_CASE("unread bytes left over mark the reader bad until they are read")
{
	const std::vector<uint8_t> data = { 1, 2 };
	ByteReader reader(data);
	CHECK(reader.readU8() == 1);
	CHECK(reader.bad());
	CHECK(reader.readU8() == 2);
	CHECK_FALSE(reader.bad());
}

TEST_CASE("a reader over part of a buffer sees only that part")
{
	const std::vector<uint8_t> data = { 9, 0x34, 0x12, 9 };
	ByteReader reader(data.data() + 1, 2);
	CHECK(reader.readU16() == 0x1234);
	CHECK_FALSE(reader.bad());
	CHECK(reader.readU8() == 0);
	CHECK(reader.bad());
}
