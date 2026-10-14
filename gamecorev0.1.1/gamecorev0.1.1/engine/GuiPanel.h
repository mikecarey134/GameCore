#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <dimension2d.h>
#include <rect.h>

namespace irr
{
	struct SEvent;
	namespace gui { class IGUIEnvironment; class IGUIElement; class IGUIButton; class IGUIEditBox; class IGUIComboBox; class IGUIListBox; }
}

class GuiPanel
{
public:
	GuiPanel(irr::gui::IGUIEnvironment* gui, irr::core::dimension2d<irr::u32> screen);
	~GuiPanel();

	irr::core::rect<irr::s32> at(const irr::core::rect<irr::s32>& design) const;

	irr::gui::IGUIButton* button(const irr::core::rect<irr::s32>& design, irr::s32 id, const std::string& text);
	irr::gui::IGUIEditBox* textBox(const irr::core::rect<irr::s32>& design, irr::s32 id, const std::string& text,
		irr::u32 maxLength, bool focus);
	irr::gui::IGUIButton* toggle(const irr::core::rect<irr::s32>& design, irr::s32 id, const std::vector<std::string>& options);
	irr::gui::IGUIComboBox* dropdown(const irr::core::rect<irr::s32>& design, irr::s32 id, const std::vector<std::string>& items,
		irr::u32 visibleRows);
	irr::gui::IGUIListBox* list(const irr::core::rect<irr::s32>& design, irr::s32 id, const std::vector<std::string>& items,
		irr::s32 selected);

	bool handle(const irr::SEvent& event);
	std::string text(irr::s32 id) const;
	irr::s32 selected(irr::s32 id) const;
	void clear();

private:
	irr::gui::IGUIElement* root();
	irr::gui::IGUIElement* find(irr::s32 id) const;

	irr::gui::IGUIEnvironment* gui_;
	irr::core::dimension2d<irr::u32> screen_;
	irr::gui::IGUIElement* root_;
	std::map<irr::s32, std::pair<std::vector<std::string>, irr::s32> > toggles_;
};

#endif
