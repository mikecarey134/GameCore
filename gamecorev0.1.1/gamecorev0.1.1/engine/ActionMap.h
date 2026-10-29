#ifndef ACTIONMAP_H
#define ACTIONMAP_H

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <IEventReceiver.h>
#include "InputState.h"

class ActionMap
{
public:
	typedef std::function<void(const std::string&)> Logger;

	void bind(const std::string& action, irr::EKEY_CODE key);
	void load(const std::string& text, const Logger& log);
	void loadFile(const std::string& path, const Logger& log);

	bool pressed(const std::string& action, const irr::SEvent& event) const;
	bool released(const std::string& action, const irr::SEvent& event) const;
	bool held(const std::string& action, const InputState& input) const;

	const std::vector<irr::EKEY_CODE>& keys(const std::string& action) const;
	irr::EKEY_CODE key(const std::string& action) const;
	std::string keyName(const std::string& action) const;

	static bool keyFromName(const std::string& name, irr::EKEY_CODE& key);
	static std::string nameOf(irr::EKEY_CODE key);

private:
	bool matches(const std::string& action, const irr::SEvent& event, bool down) const;

	std::map<std::string, std::vector<irr::EKEY_CODE> > bindings_;
};

#endif
