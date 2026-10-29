#include "ActionMap.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

using namespace irr;

namespace
{
	struct KeyName
	{
		std::string name;
		EKEY_CODE key;
		std::string display;
	};

	std::vector<KeyName> buildKeyNames()
	{
		std::vector<KeyName> names;
		for (int i = 0; i < 26; ++i)
		{
			std::string letter(1, (char)('A' + i));
			names.push_back(KeyName{ letter, (EKEY_CODE)(KEY_KEY_A + i), letter });
		}
		for (int i = 0; i < 10; ++i)
		{
			std::string digit(1, (char)('0' + i));
			names.push_back(KeyName{ digit, (EKEY_CODE)(KEY_KEY_0 + i), digit });
		}
		for (int i = 0; i < 12; ++i)
		{
			std::string function = "F" + std::to_string(i + 1);
			names.push_back(KeyName{ function, (EKEY_CODE)(KEY_F1 + i), function });
		}
		const KeyName named[] = {
			{ "Space", KEY_SPACE, "Space" }, { "Escape", KEY_ESCAPE, "Escape" }, { "Grave", KEY_OEM_3, "Grave" },
			{ "Tab", KEY_TAB, "Tab" }, { "Return", KEY_RETURN, "Return" }, { "Back", KEY_BACK, "Back" },
			{ "Delete", KEY_DELETE, "Delete" }, { "Up", KEY_UP, "Up" }, { "Down", KEY_DOWN, "Down" },
			{ "Left", KEY_LEFT, "Left" }, { "Right", KEY_RIGHT, "Right" }, { "Shift", KEY_SHIFT, "Shift" },
			{ "Ctrl", KEY_CONTROL, "Ctrl" }, { "Alt", KEY_MENU, "Alt" },
			{ "MouseLeft", KEY_LBUTTON, "Mouse Left" }, { "MouseRight", KEY_RBUTTON, "Mouse Right" },
			{ "MouseMiddle", KEY_MBUTTON, "Mouse Middle" } };
		names.insert(names.end(), named, named + sizeof(named) / sizeof(named[0]));
		return names;
	}

	const std::vector<KeyName>& keyNames()
	{
		static const std::vector<KeyName> names = buildKeyNames();
		return names;
	}

	std::vector<EKEY_CODE> variants(EKEY_CODE key)
	{
		if (key == KEY_SHIFT)
			return { KEY_SHIFT, KEY_LSHIFT, KEY_RSHIFT };
		if (key == KEY_CONTROL)
			return { KEY_CONTROL, KEY_LCONTROL, KEY_RCONTROL };
		if (key == KEY_MENU)
			return { KEY_MENU, KEY_LMENU, KEY_RMENU };
		return { key };
	}

	bool mouseButton(EMOUSE_INPUT_EVENT event, EKEY_CODE& key, bool& down)
	{
		switch (event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN: key = KEY_LBUTTON; down = true; return true;
		case EMIE_RMOUSE_PRESSED_DOWN: key = KEY_RBUTTON; down = true; return true;
		case EMIE_MMOUSE_PRESSED_DOWN: key = KEY_MBUTTON; down = true; return true;
		case EMIE_LMOUSE_LEFT_UP: key = KEY_LBUTTON; down = false; return true;
		case EMIE_RMOUSE_LEFT_UP: key = KEY_RBUTTON; down = false; return true;
		case EMIE_MMOUSE_LEFT_UP: key = KEY_MBUTTON; down = false; return true;
		default: return false;
		}
	}

	std::string trim(const std::string& text)
	{
		size_t first = text.find_first_not_of(" \t\r");
		if (first == std::string::npos)
			return "";
		return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
	}

	std::string lower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });
		return text;
	}
}

void ActionMap::bind(const std::string& action, EKEY_CODE key)
{
	bindings_[action].push_back(key);
}

void ActionMap::load(const std::string& text, const Logger& log)
{
	std::istringstream lines(text);
	std::string line;
	int number = 0;
	while (std::getline(lines, line))
	{
		++number;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		std::string where = " on key bindings line " + std::to_string(number);
		size_t equals = line.find('=');
		if (equals == std::string::npos)
		{
			log("No '=' in \"" + line + "\"" + where);
			continue;
		}
		std::string action = trim(line.substr(0, equals));
		if (bindings_.find(action) == bindings_.end())
		{
			log("Unknown action \"" + action + "\"" + where);
			continue;
		}
		std::vector<EKEY_CODE> keys;
		std::istringstream list(line.substr(equals + 1));
		std::string name;
		bool valid = true;
		while (valid && std::getline(list, name, ','))
		{
			EKEY_CODE key;
			valid = keyFromName(trim(name), key);
			if (valid)
				keys.push_back(key);
			else
				log("Unknown key \"" + trim(name) + "\" for " + action + where);
		}
		if (valid && !keys.empty())
			bindings_[action] = keys;
		else if (valid)
			log("No keys for " + action + where);
	}

	std::map<EKEY_CODE, std::vector<std::string> > users;
	for (std::map<std::string, std::vector<EKEY_CODE> >::const_iterator binding = bindings_.begin(); binding != bindings_.end(); ++binding)
		for (size_t i = 0; i < binding->second.size(); ++i)
			users[binding->second[i]].push_back(binding->first);
	for (std::map<EKEY_CODE, std::vector<std::string> >::const_iterator user = users.begin(); user != users.end(); ++user)
		if (user->second.size() > 1)
		{
			std::string actions = user->second[0];
			for (size_t i = 1; i < user->second.size(); ++i)
				actions += " and " + user->second[i];
			log("Key " + nameOf(user->first) + " is bound to " + actions);
		}
}

void ActionMap::loadFile(const std::string& path, const Logger& log)
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
	{
		log("No key bindings file at " + path + ", using the default keys");
		return;
	}
	std::ostringstream text;
	text << file.rdbuf();
	load(text.str(), log);
}

bool ActionMap::matches(const std::string& action, const SEvent& event, bool down) const
{
	EKEY_CODE key;
	if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		if (event.KeyInput.PressedDown != down)
			return false;
		key = event.KeyInput.Key;
	}
	else if (event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		bool buttonDown;
		if (!mouseButton(event.MouseInput.Event, key, buttonDown) || buttonDown != down)
			return false;
	}
	else
		return false;

	const std::vector<EKEY_CODE>& bound = keys(action);
	for (size_t i = 0; i < bound.size(); ++i)
	{
		std::vector<EKEY_CODE> sides = variants(bound[i]);
		if (std::find(sides.begin(), sides.end(), key) != sides.end())
			return true;
	}
	return false;
}

bool ActionMap::pressed(const std::string& action, const SEvent& event) const
{
	return matches(action, event, true);
}

bool ActionMap::released(const std::string& action, const SEvent& event) const
{
	return matches(action, event, false);
}

bool ActionMap::held(const std::string& action, const InputState& input) const
{
	const std::vector<EKEY_CODE>& bound = keys(action);
	for (size_t i = 0; i < bound.size(); ++i)
	{
		std::vector<EKEY_CODE> sides = variants(bound[i]);
		for (size_t j = 0; j < sides.size(); ++j)
			if (input.isKeyDown(sides[j]))
				return true;
	}
	return false;
}

const std::vector<EKEY_CODE>& ActionMap::keys(const std::string& action) const
{
	static const std::vector<EKEY_CODE> none;
	std::map<std::string, std::vector<EKEY_CODE> >::const_iterator binding = bindings_.find(action);
	return binding == bindings_.end() ? none : binding->second;
}

EKEY_CODE ActionMap::key(const std::string& action) const
{
	const std::vector<EKEY_CODE>& bound = keys(action);
	return bound.empty() ? KEY_KEY_CODES_COUNT : bound[0];
}

std::string ActionMap::keyName(const std::string& action) const
{
	const std::vector<EKEY_CODE>& bound = keys(action);
	return bound.empty() ? "" : nameOf(bound[0]);
}

bool ActionMap::keyFromName(const std::string& name, EKEY_CODE& key)
{
	const std::vector<KeyName>& names = keyNames();
	for (size_t i = 0; i < names.size(); ++i)
		if (lower(names[i].name) == lower(name))
		{
			key = names[i].key;
			return true;
		}
	return false;
}

std::string ActionMap::nameOf(EKEY_CODE key)
{
	const std::vector<KeyName>& names = keyNames();
	for (size_t i = 0; i < names.size(); ++i)
		if (names[i].key == key)
			return names[i].display;
	return "";
}
