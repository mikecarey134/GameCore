#include "InventoryScreen.h"

using namespace inventory;
using irr::video::SColor;

namespace
{
	const int cellSize = 52;
	const char* const iconFolder = "inventory/icons/";

	irr::core::rect<irr::s32> toIrr(const Rect& area)
	{
		return irr::core::rect<irr::s32>(area.x, area.y, area.x + area.width, area.y + area.height);
	}

	SColor categoryColour(Category category)
	{
		switch (category)
		{
		case Category::Weapon: return SColor(255, 214, 120, 52);
		case Category::Armor: return SColor(255, 74, 128, 214);
		case Category::Consumable: return SColor(255, 84, 176, 92);
		case Category::KeyItem: return SColor(255, 222, 196, 64);
		case Category::Tool: return SColor(255, 56, 170, 170);
		case Category::Accessory: return SColor(255, 152, 94, 204);
		}
		return SColor(255, 190, 190, 190);
	}

	const char* categoryName(Category category)
	{
		switch (category)
		{
		case Category::Weapon: return "Weapon";
		case Category::Armor: return "Armour";
		case Category::Consumable: return "Consumable";
		case Category::KeyItem: return "Key item";
		case Category::Tool: return "Tool";
		case Category::Accessory: return "Accessory";
		}
		return "";
	}

	SColor shade(SColor colour, float amount, int alpha)
	{
		return SColor(alpha, (irr::u32)(colour.getRed() * amount), (irr::u32)(colour.getGreen() * amount),
			(irr::u32)(colour.getBlue() * amount));
	}

	std::vector<Cell> dollPositions(const Catalog& catalog)
	{
		std::vector<Cell> positions;
		for (int index = 0; index < catalog.slotCount(); ++index)
			positions.push_back(Cell{ catalog.slot(index).dollColumn, catalog.slot(index).dollRow });
		return positions;
	}

	irr::core::dimension2d<irr::u32> screenSize(irr::IrrlichtDevice* device)
	{
		return device->getVideoDriver()->getScreenSize();
	}
}

InventoryScreen::InventoryScreen(irr::IrrlichtDevice* device, Inventory& inventory, const Catalog& catalog)
	: device_(device), inventory_(&inventory), catalog_(&catalog),
	layout_((int)screenSize(device).Width, (int)screenSize(device).Height, dollPositions(catalog), 8, 5),
	controller_(new InventoryController(inventory, catalog)), mouseX_(0), mouseY_(0)
{
}

InventoryScreen::InventoryScreen(irr::IrrlichtDevice* device, const std::string& loadError)
	: device_(device), inventory_(nullptr), catalog_(nullptr),
	layout_((int)screenSize(device).Width, (int)screenSize(device).Height, std::vector<Cell>(), 8, 5),
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
	drawFrame();
	if (!controller_)
	{
		drawText("Inventory unavailable: " + loadError_, layout_.statusLine(), SColor(255, 220, 90, 80));
		return;
	}
	drawDoll();
	for (int index = 0; index < catalog_->slotCount(); ++index)
		drawSlot(index);
	drawGrid();
	drawDetails();
	drawStatus();
	drawText("Left click: pick up / place      Right click: cancel      " + closeKeyName_ + ": close", layout_.hintLine(),
		SColor(255, 120, 112, 104));
	drawHeld();
}

void InventoryScreen::drawFrame()
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	irr::core::dimension2d<irr::u32> screen = driver->getScreenSize();
	driver->draw2DRectangle(SColor(170, 0, 0, 0), irr::core::rect<irr::s32>(0, 0, (irr::s32)screen.Width, (irr::s32)screen.Height));

	Rect panel = layout_.panel();
	driver->draw2DRectangle(toIrr(panel), SColor(250, 34, 29, 32), SColor(250, 34, 29, 32),
		SColor(250, 11, 9, 11), SColor(250, 11, 9, 11));
	driver->draw2DRectangleOutline(toIrr(panel), SColor(255, 104, 86, 66));
	driver->draw2DRectangleOutline(toIrr(Rect{ panel.x + 4, panel.y + 4, panel.width - 8, panel.height - 8 }),
		SColor(255, 52, 42, 36));

	irr::video::ITexture* title = driver->getTexture("bill/inv.png");
	if (title)
		driver->draw2DImage(title, toIrr(layout_.title()), irr::core::rect<irr::s32>(0, 0, 303, 72), 0, 0, true);
}

void InventoryScreen::drawInset(const Rect& area, SColor top, SColor bottom)
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	driver->draw2DRectangle(toIrr(area), top, top, bottom, bottom);
	SColor dark(255, 6, 5, 6);
	SColor light(255, 78, 68, 60);
	int right = area.x + area.width - 1;
	int bottomEdge = area.y + area.height - 1;
	driver->draw2DLine(irr::core::position2d<irr::s32>(area.x, area.y), irr::core::position2d<irr::s32>(right, area.y), dark);
	driver->draw2DLine(irr::core::position2d<irr::s32>(area.x, area.y), irr::core::position2d<irr::s32>(area.x, bottomEdge), dark);
	driver->draw2DLine(irr::core::position2d<irr::s32>(area.x, bottomEdge), irr::core::position2d<irr::s32>(right, bottomEdge), light);
	driver->draw2DLine(irr::core::position2d<irr::s32>(right, area.y), irr::core::position2d<irr::s32>(right, bottomEdge), light);
}

void InventoryScreen::drawDoll()
{
	Rect area = layout_.dollArea();
	drawInset(area, SColor(235, 14, 12, 15), SColor(235, 6, 5, 7));
	int size = area.width - 20;
	drawIcon("doll.png", Rect{ area.x + 10, area.y + (area.height - size) / 2, size, size }, SColor(255, 36, 33, 38));
}

void InventoryScreen::drawSlot(int index)
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	Rect area = layout_.slot(index);
	if (area.width == 0)
		return;
	const Slot& slot = catalog_->slot(index);
	bool blocked = inventory_->isBlocked(slot.id);
	Hit hover = layout_.hitTest(mouseX_, mouseY_);
	bool hovered = hover.kind == Hit::Kind::Slot && hover.slot == index;

	if (blocked)
		drawInset(area, SColor(255, 60, 14, 12), SColor(255, 30, 6, 6));
	else
		drawInset(area, SColor(255, 30, 27, 31), SColor(255, 15, 13, 16));

	Rect iconArea{ area.x + 10, area.y + 10, area.width - 20, area.height - 20 };
	const Item* item = inventory_->equippedIn(slot.id);
	if (item)
	{
		const ItemType* type = catalog_->itemType(item->type);
		driver->draw2DRectangleOutline(toIrr(Rect{ area.x + 2, area.y + 2, area.width - 4, area.height - 4 }),
			shade(categoryColour(type->category), 0.8f, 255));
		drawIcon(type->icon, iconArea, categoryColour(type->category));
	}
	else if (!slot.icon.empty())
		drawIcon(slot.icon, iconArea, blocked ? SColor(120, 140, 40, 36) : SColor(70, 150, 146, 156));

	if (blocked)
	{
		SColor red(255, 200, 48, 40);
		driver->draw2DLine(irr::core::position2d<irr::s32>(area.x + 8, area.y + 8),
			irr::core::position2d<irr::s32>(area.x + area.width - 8, area.y + area.height - 8), red);
		driver->draw2DLine(irr::core::position2d<irr::s32>(area.x + area.width - 8, area.y + 8),
			irr::core::position2d<irr::s32>(area.x + 8, area.y + area.height - 8), red);
		driver->draw2DRectangleOutline(toIrr(area), red);
	}
	if (hovered)
		driver->draw2DRectangleOutline(toIrr(Rect{ area.x - 1, area.y - 1, area.width + 2, area.height + 2 }),
			SColor(255, 226, 186, 96));

	drawText(blocked ? slot.name + " - Blocked" : slot.name, Rect{ area.x - 8, area.y + area.height + 3, area.width + 16, 12 },
		blocked ? SColor(255, 210, 70, 60) : SColor(255, 150, 140, 130));
}

void InventoryScreen::drawGrid()
{
	for (int row = 0; row < 5; ++row)
		for (int column = 0; column < 8; ++column)
		{
			Rect cell = layout_.cell(Cell{ column, row });
			drawInset(Rect{ cell.x + 1, cell.y + 1, cell.width - 2, cell.height - 2 },
				SColor(255, 22, 20, 23), SColor(255, 12, 11, 13));
		}

	Hit hover = layout_.hitTest(mouseX_, mouseY_);
	const Item* hoveredItem = hover.kind == Hit::Kind::Cell ? inventory_->itemAt(hover.cell) : nullptr;
	for (const Placed& placed : inventory_->backpackContents())
	{
		bool held = controller_->isHolding() && controller_->heldFrom().column == placed.anchor.column
			&& controller_->heldFrom().row == placed.anchor.row;
		bool hovered = !controller_->isHolding() && hoveredItem == inventory_->itemAt(placed.anchor);
		drawTile(placed, held, hovered);
	}
}

void InventoryScreen::drawTile(const Placed& placed, bool dimmed, bool hovered)
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	const ItemType* type = catalog_->itemType(placed.item.type);
	Rect area = layout_.footprint(placed.anchor, type->width, type->height);
	Rect tile{ area.x + 3, area.y + 3, area.width - 6, area.height - 6 };
	SColor colour = categoryColour(type->category);
	int alpha = dimmed ? 70 : 255;

	driver->draw2DRectangle(toIrr(tile), shade(colour, 0.52f, alpha), shade(colour, 0.52f, alpha),
		shade(colour, 0.26f, alpha), shade(colour, 0.26f, alpha));
	driver->draw2DRectangleOutline(toIrr(tile), hovered ? SColor(255, 236, 196, 104) : shade(colour, 0.9f, alpha));

	int side = (tile.width < tile.height ? tile.width : tile.height) - 6;
	Rect icon{ tile.x + (tile.width - side) / 2, tile.y + (tile.height - side) / 2, side, side };
	drawIcon(type->icon, icon, SColor(alpha, 236, 230, 220));

	if (placed.item.count > 1)
	{
		Rect badge{ tile.x + tile.width - 22, tile.y + tile.height - 15, 20, 13 };
		driver->draw2DRectangle(SColor(alpha * 4 / 5, 0, 0, 0), toIrr(badge));
		drawText(std::to_string(placed.item.count), Rect{ badge.x + 6, badge.y + 1, badge.width - 6, badge.height },
			SColor(alpha, 255, 240, 200));
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

	Hit target = layout_.hitTest(mouseX_, mouseY_);
	if (target.kind == Hit::Kind::Cell)
	{
		Cell anchor{ target.cell.column - grab.column, target.cell.row - grab.row };
		device_->getVideoDriver()->draw2DRectangleOutline(
			toIrr(layout_.footprint(anchor, type->width, type->height)), SColor(255, 236, 226, 200));
	}

	Rect ghost{ mouseX_ - grab.column * cellSize - cellSize / 2, mouseY_ - grab.row * cellSize - cellSize / 2,
		type->width * cellSize, type->height * cellSize };
	SColor colour = categoryColour(type->category);
	device_->getVideoDriver()->draw2DRectangle(shade(colour, 0.5f, 150), toIrr(ghost));
	int side = (ghost.width < ghost.height ? ghost.width : ghost.height) - 12;
	drawIcon(type->icon, Rect{ ghost.x + (ghost.width - side) / 2, ghost.y + (ghost.height - side) / 2, side, side },
		SColor(230, 250, 246, 236));
}

const ItemType* InventoryScreen::hoveredType(int& count) const
{
	count = 0;
	const Item* item = nullptr;
	if (controller_->isHolding())
		item = inventory_->itemAt(controller_->heldFrom());
	else
	{
		Hit hover = layout_.hitTest(mouseX_, mouseY_);
		if (hover.kind == Hit::Kind::Cell)
			item = inventory_->itemAt(hover.cell);
		else if (hover.kind == Hit::Kind::Slot)
			item = inventory_->equippedIn(catalog_->slotId(hover.slot));
	}
	if (!item)
		return nullptr;
	count = item->count;
	return catalog_->itemType(item->type);
}

std::string InventoryScreen::slotNames(const std::vector<std::string>& slotIds) const
{
	std::string names;
	for (const std::string& id : slotIds)
		for (int index = 0; index < catalog_->slotCount(); ++index)
			if (catalog_->slotId(index) == id)
				names += (names.empty() ? "" : ", ") + catalog_->slotName(index);
	return names;
}

void InventoryScreen::drawDetails()
{
	Rect area = layout_.details();
	drawInset(area, SColor(235, 20, 18, 21), SColor(235, 10, 9, 11));
	int count = 0;
	const ItemType* type = hoveredType(count);
	if (!type)
	{
		drawText("Point at an item to inspect it.", Rect{ area.x + 12, area.y + 38, area.width - 24, 14 },
			SColor(255, 96, 90, 84));
		return;
	}
	SColor colour = categoryColour(type->category);
	Rect iconBox{ area.x + 10, area.y + 10, 72, 72 };
	drawInset(iconBox, shade(colour, 0.45f, 255), shade(colour, 0.22f, 255));
	drawIcon(type->icon, Rect{ iconBox.x + 8, iconBox.y + 8, 56, 56 }, SColor(255, 240, 234, 224));

	int textX = iconBox.x + iconBox.width + 12;
	int textWidth = area.x + area.width - textX - 10;
	drawText(type->name, Rect{ textX, area.y + 8, textWidth, 24 }, SColor(255, 238, 228, 210), true);
	drawText(std::string(categoryName(type->category)) + "   " + std::to_string(type->width) + " x "
		+ std::to_string(type->height), Rect{ textX, area.y + 36, textWidth, 14 }, colour);

	std::string use = type->slots.empty() ? "Can't be equipped" : "Equips to: " + slotNames(type->slots);
	if (!type->blocks.empty())
		use += "   (two-handed: blocks " + slotNames(type->blocks) + ")";
	drawText(use, Rect{ textX, area.y + 54, textWidth, 14 }, SColor(255, 168, 160, 150));
	if (type->maxStack > 1)
		drawText("Stack " + std::to_string(count) + " / " + std::to_string(type->maxStack),
			Rect{ textX, area.y + 70, textWidth, 14 }, SColor(255, 168, 160, 150));
}

void InventoryScreen::drawStatus()
{
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	Rect line = layout_.statusLine();
	int used = controller_->cellsUsed();
	Rect bar{ line.x, line.y + 5, 120, 10 };
	drawInset(bar, SColor(255, 14, 12, 14), SColor(255, 8, 7, 8));
	driver->draw2DRectangle(SColor(255, 150, 122, 70),
		toIrr(Rect{ bar.x + 1, bar.y + 1, (bar.width - 2) * used / 40, bar.height - 2 }));
	drawText(std::to_string(used) + " / 40 cells", Rect{ bar.x + bar.width + 8, line.y + 4, 90, 14 },
		SColor(255, 176, 166, 152));
	if (!controller_->message().empty())
		drawText(controller_->message(), Rect{ line.x + 230, line.y + 4, line.width - 230, 14 }, SColor(255, 236, 96, 76));
}

void InventoryScreen::drawIcon(const std::string& icon, const Rect& area, SColor tint)
{
	if (icon.empty())
		return;
	irr::video::IVideoDriver* driver = device_->getVideoDriver();
	irr::video::ITexture* texture = driver->getTexture((std::string(iconFolder) + icon).c_str());
	if (!texture)
		return;
	irr::core::dimension2d<irr::u32> size = texture->getOriginalSize();
	SColor tints[4] = { tint, tint, tint, tint };
	driver->draw2DImage(texture, toIrr(area),
		irr::core::rect<irr::s32>(0, 0, (irr::s32)size.Width, (irr::s32)size.Height), 0, tints, true);
}

void InventoryScreen::drawText(const std::string& text, const Rect& area, SColor colour, bool big)
{
	irr::gui::IGUIEnvironment* gui = device_->getGUIEnvironment();
	irr::gui::IGUIFont* font = big ? gui->getFont("bill/bigfont.png") : nullptr;
	if (!font)
		font = gui->getBuiltInFont();
	irr::core::rect<irr::s32> clip = toIrr(area);
	font->draw(irr::core::stringw(text.c_str()), clip, colour, false, false, &clip);
}
