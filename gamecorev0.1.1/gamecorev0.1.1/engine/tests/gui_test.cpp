#include "doctest.h"
#include "GuiInputLayer.h"
#include "GuiPanel.h"
#include <irrlicht.h>
#include <string>
#include <vector>

using namespace irr;
using namespace core;
using namespace gui;

namespace
{
	struct Gui
	{
		Gui() : device(createDevice(video::EDT_NULL, dimension2d<u32>(800, 600))), env(device->getGUIEnvironment()) {}
		~Gui()
		{
			device->setEventReceiver(0);
			device->drop();
		}

		u32 elementCount() const { return env->getRootGUIElement()->getChildren().size(); }

		void press(EKEY_CODE key, wchar_t character, bool down = true)
		{
			SEvent event;
			event.EventType = EET_KEY_INPUT_EVENT;
			event.KeyInput.Key = key;
			event.KeyInput.Char = character;
			event.KeyInput.PressedDown = down;
			event.KeyInput.Shift = false;
			event.KeyInput.Control = false;
			device->postEventFromUser(event);
		}

		IrrlichtDevice* device;
		IGUIEnvironment* env;
	};

	struct Below : InputLayer
	{
		bool onEvent(const SEvent& event) override
		{
			if (event.EventType == EET_KEY_INPUT_EVENT || event.EventType == EET_MOUSE_INPUT_EVENT)
				++seen;
			return false;
		}
		int seen = 0;
	};

	SEvent clickOn(IGUIElement* element)
	{
		SEvent event;
		event.EventType = EET_GUI_EVENT;
		event.GUIEvent.Caller = element;
		event.GUIEvent.Element = 0;
		event.GUIEvent.EventType = EGET_BUTTON_CLICKED;
		return event;
	}
}

TEST_CASE("gui panel: widgets sit in one container that clear removes")
{
	Gui gui;
	u32 before = gui.elementCount();
	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	panel.button(rect<s32>(10, 10, 100, 40), 1, "Play");
	panel.textBox(rect<s32>(10, 50, 200, 80), 2, "", 24, false);
	panel.toggle(rect<s32>(10, 90, 200, 120), 3, { "Male", "Female" });
	panel.dropdown(rect<s32>(10, 130, 200, 160), 4, { "Doctor", "Chef" }, 7);
	panel.list(rect<s32>(10, 170, 200, 300), 5, { "Dana", "Eli" }, 1);
	CHECK(gui.elementCount() == before + 1);
	CHECK(panel.text(1) == "Play");

	panel.clear();
	CHECK(gui.elementCount() == before);
	CHECK(panel.text(1) == "");
	CHECK(panel.selected(3) == -1);
	panel.button(rect<s32>(10, 10, 100, 40), 1, "Again");
	CHECK(gui.elementCount() == before + 1);
}

TEST_CASE("gui panel: destroying the panel removes its widgets")
{
	Gui gui;
	u32 before = gui.elementCount();
	{
		GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
		panel.button(rect<s32>(10, 10, 100, 40), 1, "Play");
		CHECK(gui.elementCount() == before + 1);
	}
	CHECK(gui.elementCount() == before);
}

TEST_CASE("gui panel: design coordinates are anchored to the middle of a bigger screen")
{
	Gui gui;
	GuiPanel panel(gui.env, dimension2d<u32>(1024, 768));
	CHECK(panel.at(rect<s32>(340, 250, 460, 290)) == rect<s32>(452, 334, 572, 374));
	CHECK(panel.button(rect<s32>(340, 250, 460, 290), 1, "Play")->getAbsolutePosition() == rect<s32>(452, 334, 572, 374));
}

TEST_CASE("gui panel: a toggle steps through its options on each click and wraps")
{
	Gui gui;
	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	IGUIButton* gender = panel.toggle(rect<s32>(10, 10, 100, 40), 3, { "Male", "Female", "Other" });
	IGUIButton* play = panel.button(rect<s32>(10, 50, 100, 80), 1, "Play");
	CHECK(panel.selected(3) == 0);
	CHECK(panel.text(3) == "Male");

	CHECK(panel.handle(clickOn(gender)));
	CHECK(panel.selected(3) == 1);
	CHECK(panel.text(3) == "Female");
	CHECK(panel.handle(clickOn(gender)));
	CHECK(panel.handle(clickOn(gender)));
	CHECK(panel.selected(3) == 0);
	CHECK(panel.text(3) == "Male");

	CHECK_FALSE(panel.handle(clickOn(play)));
	CHECK(panel.text(1) == "Play");
}

TEST_CASE("gui panel: selected reads dropdowns and lists, and -1 for anything else")
{
	Gui gui;
	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	IGUIComboBox* professions = panel.dropdown(rect<s32>(10, 10, 200, 40), 4, { "Doctor", "Chef", "Hacker" }, 7);
	panel.list(rect<s32>(10, 50, 200, 200), 5, { "Dana", "Eli" }, 1);
	panel.button(rect<s32>(10, 210, 100, 240), 1, "Play");
	CHECK(panel.selected(4) == 0);
	professions->setSelected(2);
	CHECK(panel.selected(4) == 2);
	CHECK(panel.selected(5) == 1);
	CHECK(panel.selected(1) == -1);
	CHECK(panel.selected(99) == -1);
	CHECK(professions->getMaxSelectionRows() == 7);
}

TEST_CASE("gui panel: text reads typed text and turns anything outside ASCII into '?'")
{
	Gui gui;
	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	IGUIEditBox* name = panel.textBox(rect<s32>(10, 10, 200, 40), 2, "Dana", 24, false);
	CHECK(panel.text(2) == "Dana");
	name->setText(L"Zoë Ł");
	CHECK(panel.text(2) == "Zo? ?");
}

TEST_CASE("gui input layer: a text box can take focus while the layer is up")
{
	Gui gui;
	InputRouter router;
	GuiInputLayer layer(gui.env, [](const SEvent&) { return false; }, [] {}, [] { return true; });
	router.push(&layer);
	gui.device->setEventReceiver(&router);

	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	IGUIEditBox* name = panel.textBox(rect<s32>(10, 10, 200, 40), 2, "", 24, true);
	CHECK(gui.env->getFocus() == name);
}

TEST_CASE("gui input layer: typed keys reach the focused box and not the layers below")
{
	Gui gui;
	InputRouter router;
	Below below;
	router.push(&below);
	GuiInputLayer layer(gui.env, [](const SEvent&) { return false; }, [] {}, [] { return true; });
	router.push(&layer);
	gui.device->setEventReceiver(&router);

	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	panel.textBox(rect<s32>(10, 10, 200, 40), 2, "", 24, true);
	gui.press(KEY_KEY_D, L'D');
	gui.press(KEY_KEY_D, L'D', false);
	gui.press(KEY_KEY_O, L'o');
	CHECK(panel.text(2) == "Do");
	CHECK(below.seen == 0);
}

TEST_CASE("gui input layer: Escape goes back once per press and is not typed")
{
	Gui gui;
	InputRouter router;
	int backs = 0;
	GuiInputLayer layer(gui.env, [](const SEvent&) { return false; }, [&backs] { ++backs; }, [] { return true; });
	router.push(&layer);
	gui.device->setEventReceiver(&router);

	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	panel.textBox(rect<s32>(10, 10, 200, 40), 2, "Dana", 24, true);
	gui.press(KEY_ESCAPE, 0);
	gui.press(KEY_ESCAPE, 0, false);
	CHECK(backs == 1);
	CHECK(panel.text(2) == "Dana");
}

TEST_CASE("gui input layer: gui events go to the game, which decides what is absorbed")
{
	Gui gui;
	std::vector<s32> clicked;
	GuiInputLayer layer(gui.env, [&clicked](const SEvent& event) {
		if (event.GUIEvent.EventType != EGET_BUTTON_CLICKED)
			return false;
		clicked.push_back(event.GUIEvent.Caller->getID());
		return true;
	}, [] {}, [] { return true; });

	GuiPanel panel(gui.env, dimension2d<u32>(800, 600));
	IGUIButton* play = panel.button(rect<s32>(10, 10, 100, 40), 7, "Play");
	CHECK(layer.onEvent(clickOn(play)));
	CHECK(clicked == std::vector<s32>{ 7 });
	SEvent focused = clickOn(play);
	focused.GUIEvent.EventType = EGET_ELEMENT_FOCUSED;
	CHECK_FALSE(layer.onEvent(focused));
}

TEST_CASE("gui input layer: an inactive layer lets every event through to the layers below")
{
	Gui gui;
	InputRouter router;
	Below below;
	router.push(&below);
	bool active = false;
	GuiInputLayer layer(gui.env, [](const SEvent&) { return true; }, [] {}, [&active] { return active; });
	router.push(&layer);
	gui.device->setEventReceiver(&router);

	gui.press(KEY_KEY_A, L'a');
	CHECK(below.seen == 1);
	active = true;
	gui.press(KEY_KEY_A, L'a');
	CHECK(below.seen == 1);
}
