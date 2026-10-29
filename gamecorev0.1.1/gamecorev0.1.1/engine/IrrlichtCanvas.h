#ifndef IRRLICHTCANVAS_H
#define IRRLICHTCANVAS_H

#include "Canvas.h"

namespace irr { class IrrlichtDevice; }

class IrrlichtCanvas : public Canvas
{
public:
	explicit IrrlichtCanvas(irr::IrrlichtDevice* device) : device_(device) {}

	void drawRect(const irr::core::rect<irr::s32>& area, irr::video::SColor colour) override;
	void drawGradientRect(const irr::core::rect<irr::s32>& area, irr::video::SColor top, irr::video::SColor bottom) override;
	void drawRectOutline(const irr::core::rect<irr::s32>& area, irr::video::SColor colour) override;
	void drawLine(irr::core::position2d<irr::s32> from, irr::core::position2d<irr::s32> to, irr::video::SColor colour) override;
	void drawImage(TextureHandle texture, const irr::core::rect<irr::s32>& dest, irr::video::SColor tint) override;
	void drawImage(TextureHandle texture, const irr::core::rect<irr::s32>& dest, const irr::core::rect<irr::s32>& source,
		irr::video::SColor tint) override;
	void drawText(FontHandle font, const wchar_t* text, const irr::core::rect<irr::s32>& area, irr::video::SColor colour,
		bool hcenter, bool vcenter, bool clip) override;
	irr::core::dimension2d<irr::u32> textSize(FontHandle font, const wchar_t* text) override;
	irr::core::dimension2d<irr::u32> screenSize() override;
	TextureHandle loadTexture(const char* path) override;
	FontHandle loadFont(const char* path) override;
	FontHandle builtInFont() override;

private:
	irr::IrrlichtDevice* device_;
};

#endif
