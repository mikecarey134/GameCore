#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class ByteWriter
{
public:
	void writeU8(uint8_t value);
	void writeU16(uint16_t value);
	void writeU32(uint32_t value);
	void writeF32(float value);
	bool writeString(const std::string& text, size_t maxLength);

	const std::vector<uint8_t>& data() const { return data_; }

private:
	std::vector<uint8_t> data_;
};

class ByteReader
{
public:
	ByteReader(const uint8_t* data, size_t size);
	explicit ByteReader(const std::vector<uint8_t>& data);

	uint8_t readU8();
	uint16_t readU16();
	uint32_t readU32();
	float readF32();
	std::string readString(size_t maxLength);

	bool bad() const { return failed_ || position_ != size_; }

private:
	bool take(size_t count);

	const uint8_t* data_;
	size_t size_;
	size_t position_;
	bool failed_;
};

#endif
