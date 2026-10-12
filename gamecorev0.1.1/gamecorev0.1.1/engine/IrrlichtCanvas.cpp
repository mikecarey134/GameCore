#include "IrrlichtCanvas.h"
#include <irrlicht.h>

using namespace irr;

namespace
{
	video::ITexture* toIrr(TextureHandle texture) { return reinterpret_cast<video::ITexture*>(texture); }
	gui::IGUIFont* toIrr(FontHandle font) { return reinterpret_cast<gui::IGUIFont*>(font); }
}

void IrrlichtCanvas::drawRect(const core::rect<s32>& area, video::SColor colour)
{
	device_->getVideoDriver()->draw2DRectangle(colour, area);
}

void IrrlichtCanvas::drawGradientRect(const core::rect<s32>& area, video::SColor top, video::SColor bottom)
{
	device_->getVideoDriver()->draw2DRectangle(area, top, top, bottom, bottom);
}

void IrrlichtCanvas::drawRectOutline(const core::rect<s32>& area, video::SColor colour)
{
	device_->getVideoDriver()->draw2DRectangleOutline(area, colour);
}

void IrrlichtCanvas::drawLine(core::position2d<s32> from, core::position2d<s32> to, video::SColor colour)
{
	device_->getVideoDriver()->draw2DLine(from, to, colour);
}

void IrrlichtCanvas::drawImage(TextureHandle texture, const core::rect<s32>& dest, video::SColor tint)
{
	if (!texture)
		return;
	core::dimension2d<u32> size = toIrr(texture)->getOriginalSize();
	drawImage(texture, dest, core::rect<s32>(0, 0, (s32)size.Width, (s32)size.Height), tint);
}

void IrrlichtCanvas::drawImage(TextureHandle texture, const core::rect<s32>& dest, const core::rect<s32>& source, video::SColor tint)
{
	video::SColor tints[4] = { tint, tint, tint, tint };
	device_->getVideoDriver()->draw2DImage(toIrr(texture), dest, source, 0, tints, true);
}

void IrrlichtCanvas::drawText(FontHandle font, const wchar_t* text, const core::rect<s32>& area, video::SColor colour,
	bool hcenter, bool vcenter, bool clip)
{
	if (font)
		toIrr(font)->draw(core::stringw(text), area, colour, hcenter, vcenter, clip ? &area : 0);
}

core::dimension2d<u32> IrrlichtCanvas::textSize(FontHandle font, const wchar_t* text)
{
	return font ? toIrr(font)->getDimension(text) : core::dimension2d<u32>();
}

core::dimension2d<u32> IrrlichtCanvas::screenSize()
{
	return device_->getVideoDriver()->getScreenSize();
}

TextureHandle IrrlichtCanvas::loadTexture(const char* path)
{
	return reinterpret_cast<TextureHandle>(device_->getVideoDriver()->getTexture(path));
}

FontHandle IrrlichtCanvas::loadFont(const char* path)
{
	return reinterpret_cast<FontHandle>(device_->getGUIEnvironment()->getFont(path));
}

FontHandle IrrlichtCanvas::builtInFont()
{
	return reinterpret_cast<FontHandle>(device_->getGUIEnvironment()->getBuiltInFont());
}
