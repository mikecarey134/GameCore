#include "GuiPanel.h"
#include "UiAnchor.h"
#include <IEventReceiver.h>
#include <IGUIButton.h>
#include <IGUIComboBox.h>
#include <IGUIEditBox.h>
#include <IGUIEnvironment.h>
#include <IGUIListBox.h>
#include <IGUITabControl.h>

using namespace irr;
using namespace core;
using namespace gui;

namespace
{
	std::string narrow(const wchar_t* text)
	{
		std::string result;
		for (; text && *text; ++text)
			result += *text < 128 ? (char)*text : '?';
		return result;
	}

	std::wstring wide(const std::string& text)
	{
		return std::wstring(text.begin(), text.end());
	}
}

GuiPanel::GuiPanel(IGUIEnvironment* environment, dimension2d<u32> screen) : gui_(environment), screen_(screen), root_(0)
{
}

GuiPanel::~GuiPanel()
{
	clear();
}

rect<s32> GuiPanel::at(const rect<s32>& design) const
{
	return anchored(design, 0.5f, 0.5f, screen_);
}

IGUIElement* GuiPanel::root()
{
	if (!root_)
		root_ = gui_->addTab(rect<s32>(0, 0, (s32)screen_.Width, (s32)screen_.Height));
	return root_;
}

IGUIElement* GuiPanel::find(s32 id) const
{
	return root_ ? root_->getElementFromId(id, true) : 0;
}

IGUIButton* GuiPanel::button(const rect<s32>& design, s32 id, const std::string& text)
{
	return gui_->addButton(at(design), root(), id, wide(text).c_str());
}

IGUIEditBox* GuiPanel::textBox(const rect<s32>& design, s32 id, const std::string& text, u32 maxLength, bool focus)
{
	IGUIEditBox* box = gui_->addEditBox(wide(text).c_str(), at(design), true, root(), id);
	box->setMax(maxLength);
	if (focus)
		gui_->setFocus(box);
	return box;
}

IGUIButton* GuiPanel::toggle(const rect<s32>& design, s32 id, const std::vector<std::string>& options)
{
	toggles_[id] = std::make_pair(options, 0);
	return button(design, id, options.empty() ? std::string() : options[0]);
}

IGUIComboBox* GuiPanel::dropdown(const rect<s32>& design, s32 id, const std::vector<std::string>& items, u32 visibleRows)
{
	IGUIComboBox* box = gui_->addComboBox(at(design), root(), id);
	for (const std::string& item : items)
		box->addItem(wide(item).c_str());
	box->setMaxSelectionRows(visibleRows);
	return box;
}

IGUIListBox* GuiPanel::list(const rect<s32>& design, s32 id, const std::vector<std::string>& items, s32 selected)
{
	IGUIListBox* box = gui_->addListBox(at(design), root(), id, true);
	for (const std::string& item : items)
		box->addItem(wide(item).c_str());
	box->setSelected(selected);
	return box;
}

bool GuiPanel::handle(const SEvent& event)
{
	if (event.EventType != EET_GUI_EVENT || event.GUIEvent.EventType != EGET_BUTTON_CLICKED || !event.GUIEvent.Caller)
		return false;
	std::map<s32, std::pair<std::vector<std::string>, s32> >::iterator found = toggles_.find(event.GUIEvent.Caller->getID());
	if (found == toggles_.end() || found->second.first.empty())
		return false;
	std::vector<std::string>& options = found->second.first;
	s32& index = found->second.second;
	index = (index + 1) % (s32)options.size();
	event.GUIEvent.Caller->setText(wide(options[index]).c_str());
	return true;
}

std::string GuiPanel::text(s32 id) const
{
	IGUIElement* element = find(id);
	return element ? narrow(element->getText()) : "";
}

s32 GuiPanel::selected(s32 id) const
{
	std::map<s32, std::pair<std::vector<std::string>, s32> >::const_iterator toggle = toggles_.find(id);
	if (toggle != toggles_.end())
		return toggle->second.second;
	IGUIElement* element = find(id);
	if (!element)
		return -1;
	if (element->getType() == EGUIET_COMBO_BOX)
		return static_cast<IGUIComboBox*>(element)->getSelected();
	if (element->getType() == EGUIET_LIST_BOX)
		return static_cast<IGUIListBox*>(element)->getSelected();
	return -1;
}

void GuiPanel::clear()
{
	if (root_)
		root_->remove();
	root_ = 0;
	toggles_.clear();
}
