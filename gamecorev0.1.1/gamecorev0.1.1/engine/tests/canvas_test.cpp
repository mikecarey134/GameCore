#include "doctest.h"
#include "RecordingCanvas.h"
#include "IrrlichtCanvas.h"
#include <irrlicht.h>
#include <string>

using namespace irr;
using namespace core;
using video::SColor;

TEST_CASE("canvas: a rect draw is recorded with its area and colour")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	canvas.drawRect(rect<s32>(10, 20, 110, 70), SColor(150, 1, 2, 3));
	REQUIRE(recording.calls.size() == 1);
	CHECK(recording.calls[0].kind == RecordingCanvas::RECT);
	CHECK(recording.calls[0].area == rect<s32>(10, 20, 110, 70));
	CHECK(recording.calls[0].colour == SColor(150, 1, 2, 3));
}

TEST_CASE("canvas: gradient, outline and line draws keep their own kind and colours")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	canvas.drawGradientRect(rect<s32>(0, 0, 5, 5), SColor(255, 9, 9, 9), SColor(255, 1, 1, 1));
	canvas.drawRectOutline(rect<s32>(1, 1, 4, 4), SColor(255, 7, 7, 7));
	canvas.drawLine(position2d<s32>(2, 3), position2d<s32>(8, 9), SColor(255, 5, 5, 5));
	REQUIRE(recording.calls.size() == 3);
	CHECK(recording.calls[0].kind == RecordingCanvas::GRADIENT_RECT);
	CHECK(recording.calls[0].colour == SColor(255, 9, 9, 9));
	CHECK(recording.calls[0].colour2 == SColor(255, 1, 1, 1));
	CHECK(recording.calls[1].kind == RecordingCanvas::RECT_OUTLINE);
	CHECK(recording.calls[1].area == rect<s32>(1, 1, 4, 4));
	CHECK(recording.calls[2].kind == RecordingCanvas::LINE);
	CHECK(recording.calls[2].from == position2d<s32>(2, 3));
	CHECK(recording.calls[2].to == position2d<s32>(8, 9));
}

TEST_CASE("canvas: a text draw is recorded with its content, area and alignment")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	FontHandle font = canvas.loadFont("bill/console.bmp");
	canvas.drawText(font, L"You died", rect<s32>(0, 240, 800, 300), SColor(255, 200, 0, 0), true, true);
	canvas.drawText(font, L"plain", rect<s32>(1, 2, 3, 4), SColor(255, 1, 1, 1));
	canvas.drawText(font, L"clipped", rect<s32>(1, 2, 3, 4), SColor(255, 1, 1, 1), false, true, true);
	REQUIRE(recording.calls.size() == 3);
	CHECK(recording.calls[0].kind == RecordingCanvas::TEXT);
	CHECK(recording.calls[0].text == L"You died");
	CHECK(recording.calls[0].font == font);
	CHECK(recording.calls[0].area == rect<s32>(0, 240, 800, 300));
	CHECK(recording.calls[0].colour == SColor(255, 200, 0, 0));
	CHECK(recording.calls[0].hcenter);
	CHECK(recording.calls[0].vcenter);
	CHECK_FALSE(recording.calls[0].clip);
	CHECK_FALSE(recording.calls[1].hcenter);
	CHECK_FALSE(recording.calls[1].vcenter);
	CHECK(recording.calls[2].clip);
	CHECK_FALSE(recording.calls[2].hcenter);
	CHECK(recording.calls[2].vcenter);
}

TEST_CASE("canvas: an image draw is recorded with its texture, destination and source")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	TextureHandle logo = canvas.loadTexture("bill/pauselogo.png");
	canvas.drawImage(logo, rect<s32>(290, 150, 530, 188), rect<s32>(0, 0, 240, 38));
	canvas.drawImage(logo, rect<s32>(5, 5, 37, 37), SColor(70, 255, 255, 255));
	REQUIRE(recording.calls.size() == 2);
	CHECK(recording.calls[0].kind == RecordingCanvas::IMAGE);
	CHECK(recording.calls[0].texture == logo);
	CHECK(recording.calls[0].area == rect<s32>(290, 150, 530, 188));
	CHECK(recording.calls[0].source == rect<s32>(0, 0, 240, 38));
	CHECK_FALSE(recording.calls[0].fullSource);
	CHECK(recording.calls[0].colour == CANVAS_WHITE);
	CHECK(recording.calls[1].fullSource);
	CHECK(recording.calls[1].colour == SColor(70, 255, 255, 255));
	CHECK(recording.pathOf(logo) == "bill/pauselogo.png");
}

TEST_CASE("canvas: screen and text sizes come from the recording canvas settings")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	recording.screenDimension = dimension2d<u32>(1280, 720);
	recording.textDimension = dimension2d<u32>(42, 13);
	CHECK(canvas.screenSize() == dimension2d<u32>(1280, 720));
	CHECK(canvas.textSize(canvas.builtInFont(), L"X") == dimension2d<u32>(42, 13));
}

TEST_CASE("canvas: loaded textures and fonts get distinct handles")
{
	RecordingCanvas recording;
	Canvas& canvas = recording;
	TextureHandle a = canvas.loadTexture("a.png");
	TextureHandle b = canvas.loadTexture("b.png");
	FontHandle f = canvas.loadFont("f.png");
	FontHandle g = canvas.loadFont("g.png");
	CHECK(a != nullptr);
	CHECK(a != b);
	CHECK(f != nullptr);
	CHECK(f != g);
	CHECK(canvas.builtInFont() != nullptr);
	CHECK(canvas.builtInFont() != f);
	CHECK(canvas.builtInFont() == canvas.builtInFont());
	CHECK(recording.pathOf(b) == "b.png");
	CHECK(recording.pathOf(g) == "g.png");
}

TEST_CASE("canvas: the Irrlicht canvas loads, measures and draws on the null device")
{
	IrrlichtDevice* device = createDevice(video::EDT_NULL, dimension2d<u32>(640, 480));
	{
		IrrlichtCanvas irrlicht(device);
		Canvas& canvas = irrlicht;
		CHECK(canvas.screenSize() == dimension2d<u32>(640, 480));
		TextureHandle logo = canvas.loadTexture(GAMECORE_SOURCE_DIR "/bill/pauselogo.png");
		CHECK(logo != nullptr);
		CHECK(canvas.loadTexture(GAMECORE_SOURCE_DIR "/bill/no-such-file.png") == nullptr);
		FontHandle font = canvas.builtInFont();
		CHECK(font != nullptr);
		CHECK(font == canvas.builtInFont());
		CHECK(canvas.textSize(font, L"XX").Width > canvas.textSize(font, L"X").Width);
		CHECK(canvas.textSize(font, L"X").Height > 0);
		device->getVideoDriver()->beginScene(true, true, SColor(255, 0, 0, 0));
		canvas.drawRect(rect<s32>(0, 0, 10, 10), SColor(255, 1, 2, 3));
		canvas.drawGradientRect(rect<s32>(0, 0, 10, 10), SColor(255, 1, 2, 3), SColor(255, 4, 5, 6));
		canvas.drawRectOutline(rect<s32>(0, 0, 10, 10), SColor(255, 1, 2, 3));
		canvas.drawLine(position2d<s32>(0, 0), position2d<s32>(5, 5), SColor(255, 1, 2, 3));
		canvas.drawImage(logo, rect<s32>(0, 0, 24, 4));
		canvas.drawImage(logo, rect<s32>(0, 0, 24, 4), rect<s32>(0, 0, 240, 38));
		canvas.drawImage(nullptr, rect<s32>(0, 0, 24, 4));
		canvas.drawText(font, L"hello", rect<s32>(0, 0, 100, 20), SColor(255, 255, 255, 255), true, true, true);
		device->getVideoDriver()->endScene();
	}
	device->drop();
}
