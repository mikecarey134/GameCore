#include "InventoryScreen.h"

using namespace inventory;
using irr::video::SColor;

namespace
{
	const int cellSize = 52;

	irr::core::rect<irr::s32> toIrr(const Rect& area)
	{
		return irr::core::rect<irr::s32>(area.x, area.y, area.x + area.width, area.y + area.height);
	}

	SColor categoryColour(Category category, int alpha)
	{
		switch (category)
		{
		case Category::Weapon: return SColor(alpha, 230, 140, 60);
		case Category::Armor: return SColor(alpha, 80, 140, 230);
		case Category::Consumable: return SColor(alpha, 90, 190, 100);
		case Category::KeyItem: return SColor(alpha, 230, 210, 70);
		case Category::Tool: return SColor(alpha, 60, 180, 180);
		case Category::Accessory: return SColor(alpha, 160, 100, 210);
		}
		return SColor(alpha, 200, 200, 200);
	}

	irr::core::dimension2d<irr::u32> screenSize(irr::IrrlichtDevice* device)
	{
		return device->getVideoDriver()->getScreenSize();
	}
}

InventoryScreen::InventoryScreen(irr::IrrlichtDevice* device, Inventory& inventory, const Catalog& catalog)
	: device_(device), inventory_(&inventory), catalog_(&catalog),
	layout_((int)screenSize(device).Width, (int)screenSize(device).Height, catalog.slotCount(), 8, 5),
	controller_(new InventoryController(inventory, catalog)), mouseX_(0), mouseY_(0)
{
}

InventoryScreen::InventoryScreen(irr::IrrlichtDevice* device, const std::string& loadError)
	: device_(device), inventory_(nullptr), catalog_(nullptr),
	layout_((int)screenSize(device).Width, (int)screenSize(device).Height, 0, 8, 5),
	loadError_(loadError), mouseX_(0), mouseY_(0)
{
}

bool InventoryScreen::onEvent(const irr::SEvent& event)
{
	if (event.EventType != irr::EET_MOUSE_INPUT_EVENT)
		return false;
	mouseX_ = event.MouseInput.X;
	mouseY_ = event.MouseInput.Y;
	if (controller_)
	{
		if (event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN)
			controller_->leftClick(layout_.hitTest(mouseX_, mouseY_));
		else if (event.MouseInput.Event == irr::EMIE_RMOUSE_PRESSED_DOWN)
			controller_->rightClick();
	}
	return true;
}

void InventoryScreen::close()
{
	if (controller_)
		controller_->rightClick();
}

void InventoryScreen::draw()
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	Rect panel = layout_.panel();
	driver->draw2DRectangle(SColor(215, 18, 18, 22), toIrr(panel));
	driver->draw2DRectangleOutline(toIrr(panel), SColor(255, 90, 90, 100));

	if (!controller_)
	{
		drawText("Inventory unavailable: " + loadError_, layout_.statusLine(), SColor(255, 230, 90, 90), false);
		return;
	}
	drawSlots();
	drawGrid();
	drawHeld();
	drawStatus();
}

void InventoryScreen::drawSlots()
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	for (int index = 0; index < catalog_->slotCount(); ++index)
	{
		const std::string& slotId = catalog_->slotId(index);
		Rect area = layout_.slot(index);
		bool blocked = inventory_->isBlocked(slotId);
		driver->draw2DRectangle(blocked ? SColor(255, 40, 14, 14) : SColor(255, 34, 34, 42), toIrr(area));
		driver->draw2DRectangleOutline(toIrr(area), blocked ? SColor(255, 210, 50, 50) : SColor(255, 110, 110, 125));

		Rect label{ area.x + 6, area.y + 4, area.width - 12, 14 };
		drawText(catalog_->slotName(index) + (blocked ? " (blocked)" : ""), label, SColor(255, 170, 170, 180), false);

		Rect value{ area.x + 6, area.y + 22, area.width - 12, 16 };
		const Item* item = inventory_->equippedIn(slotId);
		if (item)
		{
			const ItemType* type = catalog_->itemType(item->type);
			drawText(type->name, value, categoryColour(type->category, 255), false);
		}
		else
			drawText("--", value, SColor(255, 100, 100, 110), false);
	}
}

void InventoryScreen::drawGrid()
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	for (int row = 0; row < 5; ++row)
		for (int column = 0; column < 8; ++column)
			driver->draw2DRectangleOutline(toIrr(layout_.cell(Cell{ column, row })), SColor(255, 60, 60, 72));

	for (const Placed& placed : inventory_->backpackContents())
	{
		const ItemType* type = catalog_->itemType(placed.item.type);
		Rect area = layout_.footprint(placed.anchor, type->width, type->height);
		Rect inner{ area.x + 2, area.y + 2, area.width - 4, area.height - 4 };
		bool held = controller_->isHolding() && controller_->heldFrom().column == placed.anchor.column
			&& controller_->heldFrom().row == placed.anchor.row;
		driver->draw2DRectangle(categoryColour(type->category, held ? 70 : 220), toIrr(inner));
		driver->draw2DRectangleOutline(toIrr(inner), SColor(255, 230, 230, 230));

		std::string size = std::to_string(type->width) + "x" + std::to_string(type->height);
		if (placed.item.count > 1)
			size += " x" + std::to_string(placed.item.count);
		drawText(type->name, Rect{ inner.x + 3, inner.y + 3, inner.width - 6, 12 }, SColor(255, 15, 15, 15), false);
		drawText(size, Rect{ inner.x + 3, inner.y + 16, inner.width - 6, 12 }, SColor(255, 30, 30, 30), false);
	}
}

void InventoryScreen::drawHeld()
{
	if (!controller_->isHolding())
		return;
	const Item* item = inventory_->itemAt(controller_->heldFrom());
	if (!item)
		return;
	const ItemType* type = catalog_->itemType(item->type);
	Cell grab = controller_->grabOffset();
	Rect ghost{ mouseX_ - grab.column * cellSize - cellSize / 2, mouseY_ - grab.row * cellSize - cellSize / 2,
		type->width * cellSize, type->height * cellSize };
	device_->getVideoDriver()->draw2DRectangle(categoryColour(type->category, 120), toIrr(ghost));
	device_->getVideoDriver()->draw2DRectangleOutline(toIrr(ghost), SColor(255, 255, 255, 255));
}

void InventoryScreen::drawStatus()
{
	Rect line = layout_.statusLine();
	drawText(std::to_string(controller_->cellsUsed()) + " / 40 cells used", line, SColor(255, 190, 190, 200), false);
	if (!controller_->message().empty())
		drawText(controller_->message(), Rect{ line.x + 200, line.y, line.width - 200, line.height },
			SColor(255, 235, 110, 90), false);
}

void InventoryScreen::drawText(const std::string& text, const Rect& area, SColor colour, bool centred)
{
	irr::gui::IGUIFont* font = device_->getGUIEnvironment()->getBuiltInFont();
	irr::core::rect<irr::s32> clip = toIrr(area);
	font->draw(irr::core::stringw(text.c_str()), clip, colour, centred, false, &clip);
}
