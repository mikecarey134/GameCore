#include "doctest.h"
#include "UiAnchor.h"
#include <string>
#include <vector>

namespace
{
	struct Log
	{
		std::vector<std::string> lines;
		std::function<void(const std::string&)> logger() { return [this](const std::string& line) { lines.push_back(line); }; }
	};

	irr::core::dimension2d<irr::u32> size(irr::u32 width, irr::u32 height) { return irr::core::dimension2d<irr::u32>(width, height); }
}

TEST_CASE("an old config without window lines opens at 800x600 without a log line")
{
	Log log;
	CHECK(windowSize({}, log.logger()) == size(800, 600));
	CHECK(windowSize({ "" }, log.logger()) == size(800, 600));
	CHECK(windowSize({ "\r" }, log.logger()) == size(800, 600));
	CHECK(log.lines.empty());
}

TEST_CASE("a config with only a width keeps the default height")
{
	Log log;
	CHECK(windowSize({ "1280" }, log.logger()) == size(1280, 600));
	CHECK(windowSize({ "1280", "\r" }, log.logger()) == size(1280, 600));
	CHECK(log.lines.empty());
}

TEST_CASE("the window lines set the window size")
{
	Log log;
	CHECK(windowSize({ "1280", "720" }, log.logger()) == size(1280, 720));
	CHECK(windowSize({ "1920\r", "1080\r" }, log.logger()) == size(1920, 1080));
	CHECK(windowSize({ "1024", "768", "" }, log.logger()) == size(1024, 768));
	CHECK(log.lines.empty());
}

TEST_CASE("a window below 800x600 is clamped up with one log line")
{
	Log log;
	CHECK(windowSize({ "640", "480" }, log.logger()) == size(800, 600));
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("640x480") != std::string::npos);
	CHECK(log.lines[0].find("800x600") != std::string::npos);
}

TEST_CASE("only the dimension below the minimum is clamped")
{
	Log log;
	CHECK(windowSize({ "1280", "500" }, log.logger()) == size(1280, 600));
	CHECK(windowSize({ "700", "900" }, log.logger()) == size(800, 900));
	CHECK(log.lines.size() == 2);
}

TEST_CASE("a window line that is not a number is clamped and logged")
{
	Log log;
	CHECK(windowSize({ "wide", "-5" }, log.logger()) == size(800, 600));
	CHECK(log.lines.size() == 1);
}
