#include "ByteStream.h"
#include <cstring>

void ByteWriter::writeU8(uint8_t value)
{
	data_.push_back(value);
}

void ByteWriter::writeU16(uint16_t value)
{
	data_.push_back((uint8_t)(value & 0xFF));
	data_.push_back((uint8_t)(value >> 8));
}

void ByteWriter::writeU32(uint32_t value)
{
	for (int shift = 0; shift < 32; shift += 8)
		data_.push_back((uint8_t)((value >> shift) & 0xFF));
}

void ByteWriter::writeF32(float value)
{
	uint32_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	writeU32(bits);
}

bool ByteWriter::writeString(const std::string& text, size_t maxLength)
{
	if (text.size() > maxLength || text.size() > 0xFFFF)
		return false;
	writeU16((uint16_t)text.size());
	data_.insert(data_.end(), text.begin(), text.end());
	return true;
}

ByteReader::ByteReader(const uint8_t* data, size_t size)
	: data_(data), size_(size), position_(0), failed_(false)
{
}

ByteReader::ByteReader(const std::vector<uint8_t>& data)
	: data_(data.data()), size_(data.size()), position_(0), failed_(false)
{
}

bool ByteReader::take(size_t count)
{
	if (failed_ || count > size_ - position_)
	{
		failed_ = true;
		return false;
	}
	position_ += count;
	return true;
}

uint8_t ByteReader::readU8()
{
	if (!take(1))
		return 0;
	return data_[position_ - 1];
}

uint16_t ByteReader::readU16()
{
	if (!take(2))
		return 0;
	const uint8_t* bytes = data_ + position_ - 2;
	return (uint16_t)(bytes[0] | (bytes[1] << 8));
}

uint32_t ByteReader::readU32()
{
	if (!take(4))
		return 0;
	const uint8_t* bytes = data_ + position_ - 4;
	return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

float ByteReader::readF32()
{
	uint32_t bits = readU32();
	float value;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

std::string ByteReader::readString(size_t maxLength)
{
	uint16_t length = readU16();
	if (length > maxLength)
		failed_ = true;
	if (!take(length))
		return "";
	return std::string((const char*)data_ + position_ - length, length);
}
