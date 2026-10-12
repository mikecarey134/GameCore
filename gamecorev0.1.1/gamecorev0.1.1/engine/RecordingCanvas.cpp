#include "RecordingCanvas.h"

using namespace irr;

RecordingCanvas::Call RecordingCanvas::blank(Kind kind) const
{
	Call call = Call();
	call.kind = kind;
	return call;
}

void RecordingCanvas::drawRect(const core::rect<s32>& area, video::SColor colour)
{
	Call call = blank(RECT);
	call.area = area;
	call.colour = colour;
	calls.push_back(call);
}

void RecordingCanvas::drawGradientRect(const core::rect<s32>& area, video::SColor top, video::SColor bottom)
{
	Call call = blank(GRADIENT_RECT);
	call.area = area;
	call.colour = top;
	call.colour2 = bottom;
	calls.push_back(call);
}

void RecordingCanvas::drawRectOutline(const core::rect<s32>& area, video::SColor colour)
{
	Call call = blank(RECT_OUTLINE);
	call.area = area;
	call.colour = colour;
	calls.push_back(call);
}

void RecordingCanvas::drawLine(core::position2d<s32> from, core::position2d<s32> to, video::SColor colour)
{
	Call call = blank(LINE);
	call.from = from;
	call.to = to;
	call.colour = colour;
	calls.push_back(call);
}

void RecordingCanvas::drawImage(TextureHandle texture, const core::rect<s32>& dest, video::SColor tint)
{
	Call call = blank(IMAGE);
	call.texture = texture;
	call.area = dest;
	call.colour = tint;
	call.fullSource = true;
	calls.push_back(call);
}

void RecordingCanvas::drawImage(TextureHandle texture, const core::rect<s32>& dest, const core::rect<s32>& source, video::SColor tint)
{
	Call call = blank(IMAGE);
	call.texture = texture;
	call.area = dest;
	call.source = source;
	call.colour = tint;
	calls.push_back(call);
}

void RecordingCanvas::drawText(FontHandle font, const wchar_t* text, const core::rect<s32>& area, video::SColor colour,
	bool hcenter, bool vcenter, bool clip)
{
	Call call = blank(TEXT);
	call.font = font;
	call.text = text;
	call.area = area;
	call.colour = colour;
	call.hcenter = hcenter;
	call.vcenter = vcenter;
	call.clip = clip;
	calls.push_back(call);
}

core::dimension2d<u32> RecordingCanvas::textSize(FontHandle, const wchar_t*)
{
	return text;
}

core::dimension2d<u32> RecordingCanvas::screenSize()
{
	return screen;
}

TextureHandle RecordingCanvas::loadTexture(const char* path)
{
	textures_.push_back(path);
	return reinterpret_cast<TextureHandle>(&textures_.back());
}

FontHandle RecordingCanvas::loadFont(const char* path)
{
	fonts_.push_back(path);
	return reinterpret_cast<FontHandle>(&fonts_.back());
}

FontHandle RecordingCanvas::builtInFont()
{
	return reinterpret_cast<FontHandle>(&builtIn_);
}

std::string RecordingCanvas::pathOf(TextureHandle texture) const
{
	return *reinterpret_cast<const std::string*>(texture);
}

std::string RecordingCanvas::pathOf(FontHandle font) const
{
	return *reinterpret_cast<const std::string*>(font);
}
