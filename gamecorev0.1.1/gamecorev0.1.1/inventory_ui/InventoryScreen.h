#pragma once
#include <IEventReceiver.h>
#include "Canvas.h"
#include <memory>
#include <string>
#include "Catalog.h"
#include "Inventory.h"
#include "InventoryController.h"
#include "InventoryLayout.h"

class InventoryScreen
{
public:
	InventoryScreen(Canvas& canvas, inventory::Inventory& inventory, const inventory::Catalog& catalog);
	InventoryScreen(Canvas& canvas, const std::string& loadError);

	bool onEvent(const irr::SEvent& event);
	void draw();
	void close();
	void setCloseKeyName(const std::string& name) { closeKeyName_ = name; }

private:
	void drawFrame();
	void drawDoll();
	void drawSlot(int index);
	void drawGrid();
	void drawTile(const inventory::Placed& placed, bool dimmed, bool hovered);
	void drawHeld();
	void drawDetails();
	void drawStatus();
	void drawIcon(const std::string& icon, const inventory::Rect& area, irr::video::SColor tint);
	void drawText(const std::string& text, const inventory::Rect& area, irr::video::SColor colour, bool big = false);
	void drawInset(const inventory::Rect& area, irr::video::SColor top, irr::video::SColor bottom);
	const inventory::ItemType* hoveredType(int& count) const;
	std::string slotNames(const std::vector<std::string>& slotIds) const;

	Canvas& canvas_;
	inventory::Inventory* inventory_;
	const inventory::Catalog* catalog_;
	inventory::InventoryLayout layout_;
	std::unique_ptr<inventory::InventoryController> controller_;
	std::string loadError_;
	std::string closeKeyName_ = "E";
	int mouseX_;
	int mouseY_;
};
