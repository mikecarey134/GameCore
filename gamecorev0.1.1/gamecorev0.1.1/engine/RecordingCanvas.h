#ifndef RECORDINGCANVAS_H
#define RECORDINGCANVAS_H

#include "Canvas.h"
#include <deque>
#include <string>
#include <vector>

class RecordingCanvas : public Canvas
{
public:
	enum Kind { RECT, GRADIENT_RECT, RECT_OUTLINE, LINE, IMAGE, TEXT };

	struct Call
	{
		Kind kind;
		irr::core::rect<irr::s32> area;
		irr::core::rect<irr::s32> source;
		irr::core::position2d<irr::s32> from;
		irr::core::position2d<irr::s32> to;
		irr::video::SColor colour;
		irr::video::SColor colour2;
		TextureHandle texture;
		FontHandle font;
		std::wstring text;
		bool hcenter;
		bool vcenter;
		bool clip;
		bool fullSource;
	};

	std::vector<Call> calls;
	irr::core::dimension2d<irr::u32> screenDimension = irr::core::dimension2d<irr::u32>(800, 600);
	irr::core::dimension2d<irr::u32> textDimension = irr::core::dimension2d<irr::u32>(6, 10);

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

	std::string pathOf(TextureHandle texture) const;
	std::string pathOf(FontHandle font) const;

private:
	Call blank(Kind kind) const;

	std::deque<std::string> textures_;
	std::deque<std::string> fonts_;
	std::string builtIn_ = "built-in";
};

#endif
