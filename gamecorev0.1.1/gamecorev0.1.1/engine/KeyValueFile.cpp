#include "KeyValueFile.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace
{
	std::string trimmed(const std::string& text)
	{
		const char* space = " \t\r\n";
		std::string::size_type first = text.find_first_not_of(space);
		if (first == std::string::npos)
			return "";
		return text.substr(first, text.find_last_not_of(space) - first + 1);
	}
}

KeyValueReader::KeyValueReader(const std::string& text)
{
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line))
	{
		line = trimmed(line);
		if (line.empty() || line[0] == '#')
			continue;
		std::string::size_type equals = line.find('=');
		if (equals == std::string::npos)
		{
			fail("line without '=': " + line);
			return;
		}
		values_[trimmed(line.substr(0, equals))] = trimmed(line.substr(equals + 1));
	}
}

bool KeyValueReader::text(const std::string& key, std::string& value)
{
	std::map<std::string, std::string>::const_iterator found = values_.find(key);
	if (found == values_.end())
		return fail("missing " + key);
	value = found->second;
	return true;
}

bool KeyValueReader::real(const std::string& key, double& value)
{
	std::string raw;
	if (!text(key, raw))
		return false;
	char* end = nullptr;
	value = std::strtod(raw.c_str(), &end);
	return (!raw.empty() && *end == '\0') || fail("bad number for " + key + ": " + raw);
}

bool KeyValueReader::whole(const std::string& key, int& value)
{
	std::string raw;
	if (!text(key, raw))
		return false;
	char* end = nullptr;
	value = (int)std::strtol(raw.c_str(), &end, 10);
	return (!raw.empty() && *end == '\0') || fail("bad whole number for " + key + ": " + raw);
}

bool KeyValueReader::fail(const std::string& message)
{
	if (problem_.empty())
		problem_ = message;
	return false;
}

void KeyValueWriter::add(const std::string& key, const std::string& value)
{
	lines_.push_back(std::make_pair(key, value));
}

void KeyValueWriter::add(const std::string& key, int value)
{
	add(key, std::to_string(value));
}

void KeyValueWriter::add(const std::string& key, double value, int digits)
{
	char buffer[64];
	std::snprintf(buffer, sizeof buffer, "%.*g", digits, value);
	add(key, std::string(buffer));
}

std::string KeyValueWriter::text() const
{
	std::string out;
	for (const std::pair<std::string, std::string>& line : lines_)
		out += line.first + " = " + line.second + "\n";
	return out;
}
