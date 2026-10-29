#include "UiAnchor.h"
#include <cstdlib>

using namespace irr;

core::rect<s32> anchored(const core::rect<s32>& design, float anchorX, float anchorY, const core::dimension2d<u32>& screen)
{
	s32 x = (s32)(((s32)screen.Width - (s32)DESIGN_WIDTH) * anchorX);
	s32 y = (s32)(((s32)screen.Height - (s32)DESIGN_HEIGHT) * anchorY);
	return core::rect<s32>(design.UpperLeftCorner.X + x, design.UpperLeftCorner.Y + y,
		design.LowerRightCorner.X + x, design.LowerRightCorner.Y + y);
}

core::rect<s32> letterboxed(const core::dimension2d<u32>& screen)
{
	s32 width = (s32)screen.Width;
	s32 height = (s32)screen.Height;
	if (width * 3 > height * 4)
		width = height * 4 / 3;
	else
		height = width * 3 / 4;
	s32 x = ((s32)screen.Width - width) / 2;
	s32 y = ((s32)screen.Height - height) / 2;
	return core::rect<s32>(x, y, x + width, y + height);
}

namespace
{
	std::string trimmed(const std::string& text)
	{
		size_t first = text.find_first_not_of(" \t\r");
		if (first == std::string::npos)
			return "";
		return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
	}
}

core::dimension2d<u32> windowSize(const std::vector<std::string>& lines, const std::function<void(const std::string&)>& log)
{
	std::string width = lines.size() > 0 ? trimmed(lines[0]) : "";
	std::string height = lines.size() > 1 ? trimmed(lines[1]) : "";
	long w = width.empty() ? (long)DESIGN_WIDTH : std::strtol(width.c_str(), 0, 10);
	long h = height.empty() ? (long)DESIGN_HEIGHT : std::strtol(height.c_str(), 0, 10);

	core::dimension2d<u32> size((u32)(w < (long)DESIGN_WIDTH ? DESIGN_WIDTH : w), (u32)(h < (long)DESIGN_HEIGHT ? DESIGN_HEIGHT : h));
	if (w < (long)DESIGN_WIDTH || h < (long)DESIGN_HEIGHT)
		log("Window size " + width + "x" + height + " is below the minimum, using " +
			std::to_string(size.Width) + "x" + std::to_string(size.Height));
	return size;
}
