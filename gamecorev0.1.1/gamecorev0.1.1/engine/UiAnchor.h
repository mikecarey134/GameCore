#ifndef UIANCHOR_H
#define UIANCHOR_H

#include <functional>
#include <string>
#include <vector>
#include <irrlicht.h>

const irr::u32 DESIGN_WIDTH = 800;
const irr::u32 DESIGN_HEIGHT = 600;

irr::core::rect<irr::s32> anchored(const irr::core::rect<irr::s32>& design, float anchorX, float anchorY,
	const irr::core::dimension2d<irr::u32>& screen);
irr::core::rect<irr::s32> letterboxed(const irr::core::dimension2d<irr::u32>& screen);
irr::core::dimension2d<irr::u32> windowSize(const std::vector<std::string>& lines,
	const std::function<void(const std::string&)>& log);

#endif
