#include "doctest.h"
#include "ActionMap.h"
#include <string>
#include <vector>

namespace
{
	irr::SEvent keyEvent(irr::EKEY_CODE key, bool pressed)
	{
		irr::SEvent event;
		event.EventType = irr::EET_KEY_INPUT_EVENT;
		event.KeyInput.Key = key;
		event.KeyInput.PressedDown = pressed;
		event.KeyInput.Char = 0;
		event.KeyInput.Shift = false;
		event.KeyInput.Control = false;
		return event;
	}

	irr::SEvent mouseEvent(irr::EMOUSE_INPUT_EVENT type)
	{
		irr::SEvent event;
		event.EventType = irr::EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = type;
		event.MouseInput.X = 0;
		event.MouseInput.Y = 0;
		event.MouseInput.Wheel = 0;
		event.MouseInput.Shift = false;
		event.MouseInput.Control = false;
		event.MouseInput.ButtonStates = 0;
		return event;
	}

	struct Log
	{
		std::vector<std::string> lines;
		ActionMap::Logger logger() { return [this](const std::string& line) { lines.push_back(line); }; }
	};

	ActionMap defaults()
	{
		ActionMap actions;
		actions.bind("move_forward", irr::KEY_KEY_W);
		actions.bind("jump", irr::KEY_SPACE);
		actions.bind("inventory", irr::KEY_KEY_E);
		actions.bind("lamp", irr::KEY_KEY_F);
		actions.bind("attack", irr::KEY_LBUTTON);
		return actions;
	}
}

TEST_CASE("defaults survive a missing bindings file, with one log line")
{
	ActionMap actions = defaults();
	Log log;
	actions.loadFile("no/such/keybinds.txt", log.logger());
	CHECK(log.lines.size() == 1);
	CHECK(actions.pressed("move_forward", keyEvent(irr::KEY_KEY_W, true)));
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
}

TEST_CASE("one bad line only affects its own action")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("jump = Nonsense\ninventory = I\n", log.logger());
	CHECK(log.lines.size() == 1);
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
	CHECK(actions.pressed("inventory", keyEvent(irr::KEY_KEY_I, true)));
	CHECK_FALSE(actions.pressed("inventory", keyEvent(irr::KEY_KEY_E, true)));
}

TEST_CASE("an unknown key name is rejected and logged by name")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("lamp = Banana", log.logger());
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("Banana") != std::string::npos);
	CHECK(actions.pressed("lamp", keyEvent(irr::KEY_KEY_F, true)));
}

TEST_CASE("one bad key in a list rejects the whole line")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("move_forward = Up, Banana", log.logger());
	CHECK(log.lines.size() == 1);
	CHECK(actions.pressed("move_forward", keyEvent(irr::KEY_KEY_W, true)));
	CHECK_FALSE(actions.pressed("move_forward", keyEvent(irr::KEY_UP, true)));
}

TEST_CASE("an unknown action is logged and skipped")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("fly = X", log.logger());
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("fly") != std::string::npos);
	CHECK(actions.keys("fly").empty());
}

TEST_CASE("a line without an equals sign is logged as such and skipped")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("jump J", log.logger());
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("'='") != std::string::npos);
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
}

TEST_CASE("an action with no keys after the equals sign is logged and keeps its default")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("jump =   ", log.logger());
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("jump") != std::string::npos);
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
}

TEST_CASE("comments, blank lines and whitespace are ignored")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("# jump = J\n\n   \r\n   inventory   =   I   \r\n  # lamp = L\n", log.logger());
	CHECK(log.lines.empty());
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
	CHECK(actions.pressed("inventory", keyEvent(irr::KEY_KEY_I, true)));
	CHECK(actions.pressed("lamp", keyEvent(irr::KEY_KEY_F, true)));
}

TEST_CASE("key names are not case sensitive")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("jump = space\ninventory = i", log.logger());
	CHECK(log.lines.empty());
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SPACE, true)));
	CHECK(actions.pressed("inventory", keyEvent(irr::KEY_KEY_I, true)));
}

TEST_CASE("pressed fires on key-down for a bound key only")
{
	ActionMap actions = defaults();
	CHECK(actions.pressed("lamp", keyEvent(irr::KEY_KEY_F, true)));
	CHECK_FALSE(actions.pressed("lamp", keyEvent(irr::KEY_KEY_F, false)));
	CHECK_FALSE(actions.pressed("lamp", keyEvent(irr::KEY_KEY_G, true)));
	CHECK_FALSE(actions.pressed("jump", keyEvent(irr::KEY_KEY_F, true)));
}

TEST_CASE("pressed and released follow a mouse button binding")
{
	ActionMap actions = defaults();
	CHECK(actions.pressed("attack", mouseEvent(irr::EMIE_LMOUSE_PRESSED_DOWN)));
	CHECK_FALSE(actions.pressed("attack", mouseEvent(irr::EMIE_RMOUSE_PRESSED_DOWN)));
	CHECK_FALSE(actions.pressed("attack", mouseEvent(irr::EMIE_LMOUSE_LEFT_UP)));
	CHECK(actions.released("attack", mouseEvent(irr::EMIE_LMOUSE_LEFT_UP)));
	CHECK_FALSE(actions.released("attack", mouseEvent(irr::EMIE_LMOUSE_PRESSED_DOWN)));
	CHECK_FALSE(actions.pressed("attack", mouseEvent(irr::EMIE_MOUSE_MOVED)));
}

TEST_CASE("released fires on key-up for a bound key only")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("attack = K", log.logger());
	CHECK(actions.released("attack", keyEvent(irr::KEY_KEY_K, false)));
	CHECK_FALSE(actions.released("attack", keyEvent(irr::KEY_KEY_K, true)));
	CHECK_FALSE(actions.released("attack", mouseEvent(irr::EMIE_LMOUSE_LEFT_UP)));
}

TEST_CASE("held is true while a bound key is down")
{
	ActionMap actions = defaults();
	InputState input;
	CHECK_FALSE(actions.held("move_forward", input));
	input.update(keyEvent(irr::KEY_KEY_W, true));
	CHECK(actions.held("move_forward", input));
	CHECK_FALSE(actions.held("jump", input));
	input.update(keyEvent(irr::KEY_KEY_W, false));
	CHECK_FALSE(actions.held("move_forward", input));
}

TEST_CASE("held follows mouse buttons through the input state")
{
	ActionMap actions = defaults();
	InputState input;
	input.update(mouseEvent(irr::EMIE_LMOUSE_PRESSED_DOWN));
	CHECK(actions.held("attack", input));
	input.update(mouseEvent(irr::EMIE_RMOUSE_PRESSED_DOWN));
	input.update(mouseEvent(irr::EMIE_LMOUSE_LEFT_UP));
	CHECK_FALSE(actions.held("attack", input));
	CHECK(input.isKeyDown(irr::KEY_RBUTTON));
	input.update(mouseEvent(irr::EMIE_MMOUSE_PRESSED_DOWN));
	CHECK(input.isKeyDown(irr::KEY_MBUTTON));
	input.update(mouseEvent(irr::EMIE_MMOUSE_LEFT_UP));
	input.update(mouseEvent(irr::EMIE_RMOUSE_LEFT_UP));
	CHECK(input.isKeyUp(irr::KEY_MBUTTON));
	CHECK(input.isKeyUp(irr::KEY_RBUTTON));
}

TEST_CASE("a rebind replaces the default key")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("move_forward = Up", log.logger());
	CHECK(actions.pressed("move_forward", keyEvent(irr::KEY_UP, true)));
	CHECK_FALSE(actions.pressed("move_forward", keyEvent(irr::KEY_KEY_W, true)));
	InputState input;
	input.update(keyEvent(irr::KEY_KEY_W, true));
	CHECK_FALSE(actions.held("move_forward", input));
}

TEST_CASE("every key in a list binding fires")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("move_forward = W, Up", log.logger());
	CHECK(log.lines.empty());
	CHECK(actions.pressed("move_forward", keyEvent(irr::KEY_KEY_W, true)));
	CHECK(actions.pressed("move_forward", keyEvent(irr::KEY_UP, true)));
	InputState input;
	input.update(keyEvent(irr::KEY_UP, true));
	CHECK(actions.held("move_forward", input));
	CHECK(actions.keys("move_forward").size() == 2);
}

TEST_CASE("one key on two actions fires both and is logged")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("lamp = E", log.logger());
	REQUIRE(log.lines.size() == 1);
	CHECK(log.lines[0].find("inventory") != std::string::npos);
	CHECK(log.lines[0].find("lamp") != std::string::npos);
	CHECK(actions.pressed("lamp", keyEvent(irr::KEY_KEY_E, true)));
	CHECK(actions.pressed("inventory", keyEvent(irr::KEY_KEY_E, true)));
}

TEST_CASE("Shift, Ctrl and Alt bindings match either side of the keyboard")
{
	ActionMap actions = defaults();
	Log log;
	actions.load("jump = Shift\nlamp = Ctrl\ninventory = Alt", log.logger());
	CHECK(log.lines.empty());
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_LSHIFT, true)));
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_RSHIFT, true)));
	CHECK(actions.pressed("jump", keyEvent(irr::KEY_SHIFT, true)));
	CHECK(actions.pressed("lamp", keyEvent(irr::KEY_LCONTROL, true)));
	CHECK(actions.pressed("inventory", keyEvent(irr::KEY_RMENU, true)));
	InputState input;
	input.update(keyEvent(irr::KEY_RSHIFT, true));
	CHECK(actions.held("jump", input));
}

TEST_CASE("key gives the first bound key and keyName its readable name")
{
	ActionMap actions = defaults();
	CHECK(actions.key("jump") == irr::KEY_SPACE);
	CHECK(actions.keyName("jump") == "Space");
	CHECK(actions.keyName("attack") == "Mouse Left");
	CHECK(actions.keyName("inventory") == "E");
	Log log;
	actions.load("move_forward = Up, W\nlamp = F3\ninventory = Grave", log.logger());
	CHECK(actions.key("move_forward") == irr::KEY_UP);
	CHECK(actions.keyName("move_forward") == "Up");
	CHECK(actions.keyName("lamp") == "F3");
	CHECK(actions.keyName("inventory") == "Grave");
	CHECK(actions.key("fly") == irr::KEY_KEY_CODES_COUNT);
	CHECK(actions.keyName("fly").empty());
}

TEST_CASE("the key name table covers letters, digits, function keys and named keys")
{
	const char* names[] = { "A", "Z", "0", "9", "F1", "F12", "Space", "Escape", "Grave", "Tab", "Return", "Back",
		"Delete", "Up", "Down", "Left", "Right", "Shift", "Ctrl", "Alt", "MouseLeft", "MouseRight", "MouseMiddle" };
	const irr::EKEY_CODE codes[] = { irr::KEY_KEY_A, irr::KEY_KEY_Z, irr::KEY_KEY_0, irr::KEY_KEY_9, irr::KEY_F1,
		irr::KEY_F12, irr::KEY_SPACE, irr::KEY_ESCAPE, irr::KEY_OEM_3, irr::KEY_TAB, irr::KEY_RETURN, irr::KEY_BACK,
		irr::KEY_DELETE, irr::KEY_UP, irr::KEY_DOWN, irr::KEY_LEFT, irr::KEY_RIGHT, irr::KEY_SHIFT, irr::KEY_CONTROL,
		irr::KEY_MENU, irr::KEY_LBUTTON, irr::KEY_RBUTTON, irr::KEY_MBUTTON };
	for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); ++i)
	{
		irr::EKEY_CODE key = irr::KEY_KEY_CODES_COUNT;
		CHECK_MESSAGE(ActionMap::keyFromName(names[i], key), names[i]);
		CHECK_MESSAGE(key == codes[i], names[i]);
	}
	irr::EKEY_CODE key;
	CHECK_FALSE(ActionMap::keyFromName("F13", key));
	CHECK_FALSE(ActionMap::keyFromName("", key));
	CHECK(ActionMap::nameOf(irr::KEY_MBUTTON) == "Mouse Middle");
	CHECK(ActionMap::nameOf(irr::KEY_OEM_3) == "Grave");
}
