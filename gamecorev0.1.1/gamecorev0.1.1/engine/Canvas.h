#ifndef CANVAS_H
#define CANVAS_H

#include <rect.h>
#include <dimension2d.h>
#include <position2d.h>
#include <SColor.h>

struct CanvasTexture;
struct CanvasFont;
typedef CanvasTexture* TextureHandle;
typedef CanvasFont* FontHandle;

const irr::video::SColor CANVAS_WHITE(255, 255, 255, 255);

class Canvas
{
public:
	virtual ~Canvas() {}

	virtual void drawRect(const irr::core::rect<irr::s32>& area, irr::video::SColor colour) = 0;
	virtual void drawGradientRect(const irr::core::rect<irr::s32>& area, irr::video::SColor top, irr::video::SColor bottom) = 0;
	virtual void drawRectOutline(const irr::core::rect<irr::s32>& area, irr::video::SColor colour) = 0;
	virtual void drawLine(irr::core::position2d<irr::s32> from, irr::core::position2d<irr::s32> to, irr::video::SColor colour) = 0;
	virtual void drawImage(TextureHandle texture, const irr::core::rect<irr::s32>& dest, irr::video::SColor tint = CANVAS_WHITE) = 0;
	virtual void drawImage(TextureHandle texture, const irr::core::rect<irr::s32>& dest, const irr::core::rect<irr::s32>& source,
		irr::video::SColor tint = CANVAS_WHITE) = 0;
	virtual void drawText(FontHandle font, const wchar_t* text, const irr::core::rect<irr::s32>& area, irr::video::SColor colour,
		bool hcenter = false, bool vcenter = false, bool clip = false) = 0;
	virtual irr::core::dimension2d<irr::u32> textSize(FontHandle font, const wchar_t* text) = 0;
	virtual irr::core::dimension2d<irr::u32> screenSize() = 0;
	virtual TextureHandle loadTexture(const char* path) = 0;
	virtual FontHandle loadFont(const char* path) = 0;
	virtual FontHandle builtInFont() = 0;
};

#endif
