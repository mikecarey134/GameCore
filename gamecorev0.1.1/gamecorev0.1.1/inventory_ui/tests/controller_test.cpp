#include "doctest.h"
#include "InventoryController.h"

using namespace inventory;

static Catalog fixture()
{
	Catalog catalog;
	LoadError error;
	bool loaded = loadCatalog(
		"id, name\n"
		"head, Head\n"
		"chest, Chest\n"
		"main_hand, Main Hand\n"
		"off_hand, Off Hand\n",
		"id, name, width, height, slots, blocks, max_stack, category\n"
		"key, Key, 1, 1, -, -, 1, key_item\n"
		"pistol, Pistol, 1, 3, main_hand, -, 1, weapon\n"
		"shotgun, Shotgun, 1, 4, main_hand, off_hand, 1, weapon\n"
		"knife, Knife, 1, 2, main_hand|off_hand, -, 1, weapon\n"
		"helmet, Helmet, 2, 2, head, -, 1, armor\n"
		"bandage, Bandage, 1, 1, -, -, 5, consumable\n"
		"ammo_box, Ammo Box, 1, 1, -, -, 10, consumable\n",
		catalog, error);
	INFO(error.file, " line ", error.line, ": ", error.message);
	REQUIRE(loaded);
	return catalog;
}

TEST_CASE("a new controller holds nothing and has no message")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	InventoryController controller(inventory, catalog);

	CHECK_FALSE(controller.isHolding());
	CHECK(controller.message().empty());
	CHECK(controller.cellsUsed() == 0);
}

TEST_CASE("cells used counts every cell each item covers, a stack counting once")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.pickUp(Item{ "pistol", 1 }).outcome == Outcome::Done);
	REQUIRE(inventory.pickUp(Item{ "key", 1 }).outcome == Outcome::Done);
	REQUIRE(inventory.pickUp(Item{ "bandage", 3 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);

	CHECK(controller.cellsUsed() == 5);
}

static Hit cellHit(int column, int row)
{
	return Hit{ Hit::Kind::Cell, Cell{ column, row }, -1 };
}

static Hit slotHit(int slot)
{
	return Hit{ Hit::Kind::Slot, Cell{ 0, 0 }, slot };
}

TEST_CASE("clicking any cell of an item holds it by its anchor and remembers where it was grabbed")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);

	controller.leftClick(cellHit(1, 2));

	REQUIRE(controller.isHolding());
	CHECK(controller.heldFrom().column == 1);
	CHECK(controller.heldFrom().row == 0);
	CHECK(controller.grabOffset().column == 0);
	CHECK(controller.grabOffset().row == 2);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].anchor.column == 1);
	CHECK(inventory.backpackContents()[0].anchor.row == 0);
}

TEST_CASE("clicking a cell while holding moves the item so the grabbed cell lands there")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(1, 2));

	controller.leftClick(cellHit(3, 3));

	CHECK_FALSE(controller.isHolding());
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].anchor.column == 3);
	CHECK(inventory.backpackContents()[0].anchor.row == 1);
}

TEST_CASE("a drop onto another item is refused, keeps holding and says why")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 1 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(1, 0));

	controller.leftClick(cellHit(3, 0));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "Something is in the way");
	CHECK(inventory.backpackContents()[0].anchor.column == 1);
}

TEST_CASE("a drop that would leave the grid is refused with its own message")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));

	controller.leftClick(cellHit(2, 2));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "It doesn't fit there");
}

TEST_CASE("right-click lets go of the held item and nothing moves")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(1, 1));

	controller.rightClick();

	CHECK_FALSE(controller.isHolding());
	CHECK(inventory.backpackContents()[0].anchor.column == 1);
	CHECK(inventory.backpackContents()[0].anchor.row == 0);
}

TEST_CASE("clicking a slot while holding equips the held item")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(1, 1));

	controller.leftClick(slotHit(2));

	CHECK_FALSE(controller.isHolding());
	REQUIRE(inventory.equippedIn("main_hand") != nullptr);
	CHECK(inventory.equippedIn("main_hand")->type == "pistol");
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("equipping into a slot the item cannot use names both")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(1, 1));

	controller.leftClick(slotHit(0));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "A Pistol can't go in Head");
	CHECK(inventory.equippedIn("head") == nullptr);
}

TEST_CASE("an item whose name starts with a vowel is called an, not a")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "ammo_box", 4 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));

	controller.leftClick(slotHit(0));

	CHECK(controller.message() == "An Ammo Box can't go in Head");
}

TEST_CASE("equipping into a blocked slot says the slot is blocked")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(2, 0));

	controller.leftClick(slotHit(3));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "Off Hand is blocked");
	CHECK(inventory.equippedIn("off_hand") == nullptr);
}

TEST_CASE("clicking an occupied slot with nothing held unequips into the backpack")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 2, 2 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 2, 2 }, "head").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);

	controller.leftClick(slotHit(0));

	CHECK_FALSE(controller.isHolding());
	CHECK(inventory.equippedIn("head") == nullptr);
	REQUIRE(inventory.backpackContents().size() == 2);
	CHECK(inventory.backpackContents()[1].item.type == "helmet");
}

TEST_CASE("unequipping into a full backpack says there is no room")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 2, 2);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "head").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 1, 1 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);

	controller.leftClick(slotHit(0));

	CHECK(controller.message() == "No room in the backpack");
	REQUIRE(inventory.equippedIn("head") != nullptr);
}

TEST_CASE("a successful drop clears the last refusal message")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));
	controller.leftClick(cellHit(2, 2));
	REQUIRE_FALSE(controller.message().empty());

	controller.leftClick(cellHit(2, 0));

	CHECK_FALSE(controller.isHolding());
	CHECK(controller.message().empty());
}

TEST_CASE("clicking outside the grid and slots while holding keeps holding and moves nothing")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 2 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(2, 2));

	controller.leftClick(Hit{ Hit::Kind::None, Cell{ 0, 0 }, -1 });

	CHECK(controller.isHolding());
	CHECK(inventory.backpackContents()[0].anchor.column == 2);
	CHECK(inventory.backpackContents()[0].anchor.row == 2);
}

TEST_CASE("with nothing held, clicking an empty cell or an empty slot does nothing")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);

	controller.leftClick(cellHit(3, 3));
	CHECK_FALSE(controller.isHolding());
	controller.leftClick(slotHit(1));
	CHECK_FALSE(controller.isHolding());
	CHECK(controller.message().empty());
	CHECK(inventory.backpackContents().size() == 1);
}

TEST_CASE("dropping onto a full stack of the same kind says the stack is full")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "bandage", 4 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "bandage", 5 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));

	controller.leftClick(cellHit(2, 0));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "That stack is full");
}

TEST_CASE("right-click clears the last refusal along with the hold")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));
	controller.leftClick(slotHit(0));
	REQUIRE_FALSE(controller.message().empty());

	controller.rightClick();

	CHECK(controller.message().empty());
}

TEST_CASE("if the held item is gone by the time it is dropped, the hold ends and says so")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 2 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(2, 2));
	REQUIRE(inventory.takeAt(Cell{ 2, 2 }, 1).outcome == Outcome::Done);

	controller.leftClick(cellHit(0, 0));

	CHECK_FALSE(controller.isHolding());
	CHECK(controller.message() == "That item is no longer there");
}

TEST_CASE("a drop that merges part of a stack ends the hold, the rest staying where it was")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "bandage", 4 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "bandage", 3 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));

	controller.leftClick(cellHit(2, 0));

	CHECK_FALSE(controller.isHolding());
	CHECK(inventory.itemAt(Cell{ 2, 0 })->count == 5);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 2);
}

TEST_CASE("an equip whose swapped-out item has no room says there is no room in the backpack")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 1, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 2 }).outcome == Outcome::Done);
	InventoryController controller(inventory, catalog);
	controller.leftClick(cellHit(0, 0));

	controller.leftClick(slotHit(2));

	CHECK(controller.isHolding());
	CHECK(controller.message() == "No room in the backpack");
	CHECK(inventory.equippedIn("main_hand")->type == "pistol");
}
