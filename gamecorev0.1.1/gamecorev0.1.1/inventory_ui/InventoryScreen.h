#pragma once
#include <irrlicht.h>
#include <memory>
#include <string>
#include "Catalog.h"
#include "Inventory.h"
#include "InventoryController.h"
#include "InventoryLayout.h"

class InventoryScreen
{
public:
	InventoryScreen(irr::IrrlichtDevice* device, inventory::Inventory& inventory, const inventory::Catalog& catalog);
	InventoryScreen(irr::IrrlichtDevice* device, const std::string& loadError);

	bool onEvent(const irr::SEvent& event);
	void draw();
	void close();

private:
	void drawSlots();
	void drawGrid();
	void drawHeld();
	void drawStatus();
	void drawText(const std::string& text, const inventory::Rect& area, irr::video::SColor colour, bool centred);

	irr::IrrlichtDevice* device_;
	inventory::Inventory* inventory_;
	const inventory::Catalog* catalog_;
	inventory::InventoryLayout layout_;
	std::unique_ptr<inventory::InventoryController> controller_;
	std::string loadError_;
	int mouseX_;
	int mouseY_;
};
