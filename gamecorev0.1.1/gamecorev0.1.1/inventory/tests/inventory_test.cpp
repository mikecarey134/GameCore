#include "doctest.h"
#include "Catalog.h"
#include "Inventory.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <type_traits>

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
		"armor, Armor, 2, 3, chest, -, 1, armor\n"
		"vest, Vest, 2, 2, chest, -, 1, armor\n"
		"bandage, Bandage, 1, 1, -, -, 5, consumable\n",
		catalog, error);
	INFO(error.file, " line ", error.line, ": ", error.message);
	REQUIRE(loaded);
	return catalog;
}

TEST_CASE("a new inventory is empty")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	CHECK(inventory.itemAt(Cell{ 0, 0 }) == nullptr);
	CHECK(inventory.itemAt(Cell{ 3, 2 }) == nullptr);
	CHECK(inventory.backpackContents().empty());
	CHECK(inventory.equippedIn("head") == nullptr);
	CHECK_FALSE(inventory.isBlocked("off_hand"));
}

TEST_CASE("picking up an item puts it in the top-left cell")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.pickUp(Item{ "key", 1 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 1);
	CHECK(result.leftover == 0);
	REQUIRE(inventory.itemAt(Cell{ 0, 0 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->type == "key");
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 1);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].anchor.column == 0);
	CHECK(inventory.backpackContents()[0].anchor.row == 0);
}

TEST_CASE("a shaped item covers its whole footprint")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	REQUIRE(inventory.pickUp(Item{ "pistol", 1 }).outcome == Outcome::Done);

	for (int row = 0; row < 3; ++row)
	{
		CAPTURE(row);
		REQUIRE(inventory.itemAt(Cell{ 0, row }) != nullptr);
		CHECK(inventory.itemAt(Cell{ 0, row })->type == "pistol");
		CHECK(inventory.itemAt(Cell{ 1, row }) == nullptr);
	}
}

TEST_CASE("picking up fills the top row left to right before the next row")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 2, 2);

	for (int i = 0; i < 4; ++i)
		REQUIRE(inventory.pickUp(Item{ "key", 1 }).outcome == Outcome::Done);

	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 4);
	const int expected[4][2] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
	for (int i = 0; i < 4; ++i)
	{
		CAPTURE(i);
		CHECK(contents[i].anchor.column == expected[i][0]);
		CHECK(contents[i].anchor.row == expected[i][1]);
	}
}

TEST_CASE("picking up skips spots where the item would overlap or leave the grid")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 3, 4);

	REQUIRE(inventory.pickUp(Item{ "key", 1 }).outcome == Outcome::Done);
	REQUIRE(inventory.pickUp(Item{ "helmet", 1 }).outcome == Outcome::Done);
	REQUIRE(inventory.pickUp(Item{ "pistol", 1 }).outcome == Outcome::Done);

	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 3);
	CHECK(contents[1].item.type == "helmet");
	CHECK(contents[1].anchor.column == 1);
	CHECK(contents[1].anchor.row == 0);
	CHECK(contents[2].item.type == "pistol");
	CHECK(contents[2].anchor.column == 0);
	CHECK(contents[2].anchor.row == 1);
}

TEST_CASE("an item with nowhere to fit is refused and the backpack is unchanged")
{
	Catalog catalog = fixture();

	SUBCASE("too tall for the grid")
	{
		Inventory inventory(catalog, 2, 2);
		Result result = inventory.pickUp(Item{ "pistol", 1 });
		CHECK(result.outcome == Outcome::NoRoom);
		CHECK(result.moved == 0);
		CHECK(result.leftover == 1);
		CHECK(inventory.backpackContents().empty());
	}
	SUBCASE("too wide for the grid")
	{
		Inventory inventory(catalog, 1, 3);
		Result result = inventory.pickUp(Item{ "helmet", 1 });
		CHECK(result.outcome == Outcome::NoRoom);
		CHECK(inventory.backpackContents().empty());
	}
	SUBCASE("the free cells are scattered")
	{
		Inventory inventory(catalog, 2, 2);
		REQUIRE(inventory.pickUp(Item{ "key", 1 }).outcome == Outcome::Done);
		Result result = inventory.pickUp(Item{ "helmet", 1 });
		CHECK(result.outcome == Outcome::NoRoom);
		REQUIRE(inventory.backpackContents().size() == 1);
		CHECK(inventory.backpackContents()[0].item.type == "key");
	}
}

TEST_CASE("an item type the catalog does not know is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.pickUp(Item{ "grenade", 1 });

	CHECK(result.outcome == Outcome::UnknownItem);
	CHECK(result.leftover == 1);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("a count below 1, or above 1 for an item that does not stack, is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	Item item;

	SUBCASE("zero keys") { item = Item{ "key", 0 }; }
	SUBCASE("negative bandages") { item = Item{ "bandage", -3 }; }
	SUBCASE("two keys at once") { item = Item{ "key", 2 }; }

	Result result = inventory.pickUp(item);

	CHECK(result.outcome == Outcome::BadCount);
	CHECK(result.moved == 0);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("picking up a stackable item tops up an existing stack first")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.pickUp(Item{ "bandage", 3 }).outcome == Outcome::Done);

	Result result = inventory.pickUp(Item{ "bandage", 2 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 2);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].item.count == 5);
}

TEST_CASE("picking up more than a stack holds splits it into several stacks")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.pickUp(Item{ "bandage", 12 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 12);
	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 3);
	CHECK(contents[0].item.count == 5);
	CHECK(contents[1].item.count == 5);
	CHECK(contents[2].item.count == 2);
	CHECK(contents[2].anchor.column == 2);
	CHECK(contents[2].anchor.row == 0);
}

TEST_CASE("a stack that only partly fits takes what fits and reports the rest")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 1, 1);
	REQUIRE(inventory.pickUp(Item{ "bandage", 4 }).outcome == Outcome::Done);

	Result result = inventory.pickUp(Item{ "bandage", 3 });

	CHECK(result.outcome == Outcome::Partial);
	CHECK(result.moved == 1);
	CHECK(result.leftover == 2);
	REQUIRE(inventory.itemAt(Cell{ 0, 0 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 5);
}

TEST_CASE("placing an item puts its anchor on the chosen cell")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.placeAt(Item{ "helmet", 1 }, Cell{ 2, 1 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 1);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].anchor.column == 2);
	CHECK(inventory.backpackContents()[0].anchor.row == 1);
	CHECK(inventory.itemAt(Cell{ 3, 2 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 1, 1 }) == nullptr);
}

TEST_CASE("placing an item whose footprint leaves the grid is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	Cell anchor{ 0, 0 };

	SUBCASE("past the right edge") { anchor = Cell{ 3, 0 }; }
	SUBCASE("past the bottom edge") { anchor = Cell{ 0, 2 }; }
	SUBCASE("left of the grid") { anchor = Cell{ -1, 0 }; }
	SUBCASE("above the grid") { anchor = Cell{ 0, -1 }; }

	Result result = inventory.placeAt(Item{ "helmet", 1 }, anchor);

	CHECK(result.outcome == Outcome::OutOfBounds);
	CHECK(result.leftover == 1);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("placing an item over another item is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);

	Result result = inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 1 });

	CHECK(result.outcome == Outcome::Overlaps);
	CHECK(result.leftover == 1);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].item.type == "pistol");
}

TEST_CASE("placing refuses unknown item types and bad counts")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	Item item;
	Outcome expected = Outcome::Done;

	SUBCASE("unknown type") { item = Item{ "grenade", 1 }; expected = Outcome::UnknownItem; }
	SUBCASE("two keys") { item = Item{ "key", 2 }; expected = Outcome::BadCount; }
	SUBCASE("no bandages") { item = Item{ "bandage", 0 }; expected = Outcome::BadCount; }

	Result result = inventory.placeAt(item, Cell{ 0, 0 });

	CHECK(result.outcome == expected);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("placing a stack onto the same kind of stack adds to it, up to its limit")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "bandage", 3 }, Cell{ 1, 1 }).outcome == Outcome::Done);

	SUBCASE("all of it fits")
	{
		Result result = inventory.placeAt(Item{ "bandage", 2 }, Cell{ 1, 1 });
		CHECK(result.outcome == Outcome::Done);
		CHECK(result.moved == 2);
		CHECK(inventory.itemAt(Cell{ 1, 1 })->count == 5);
	}
	SUBCASE("some of it fits")
	{
		Result result = inventory.placeAt(Item{ "bandage", 4 }, Cell{ 1, 1 });
		CHECK(result.outcome == Outcome::Partial);
		CHECK(result.moved == 2);
		CHECK(result.leftover == 2);
		CHECK(inventory.itemAt(Cell{ 1, 1 })->count == 5);
	}
	SUBCASE("the stack is already full")
	{
		REQUIRE(inventory.placeAt(Item{ "bandage", 2 }, Cell{ 1, 1 }).outcome == Outcome::Done);
		Result result = inventory.placeAt(Item{ "bandage", 1 }, Cell{ 1, 1 });
		CHECK(result.outcome == Outcome::NoRoom);
		CHECK(result.leftover == 1);
	}
	CHECK(inventory.backpackContents().size() == 1);
}

TEST_CASE("placing more than a stack holds on an empty cell places one full stack")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.placeAt(Item{ "bandage", 7 }, Cell{ 2, 2 });

	CHECK(result.outcome == Outcome::Partial);
	CHECK(result.moved == 5);
	CHECK(result.leftover == 2);
	REQUIRE(inventory.itemAt(Cell{ 2, 2 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 2, 2 })->count == 5);
}

TEST_CASE("items that do not stack never merge, even with their own kind")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 0 });

	CHECK(result.outcome == Outcome::Overlaps);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 1);
}

TEST_CASE("taking an item from any cell it covers removes it and frees its footprint")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);

	Taken taken = inventory.takeAt(Cell{ 2, 2 }, 1);

	CHECK(taken.outcome == Outcome::Done);
	CHECK(taken.item.type == "pistol");
	CHECK(taken.item.count == 1);
	CHECK(inventory.backpackContents().empty());
	CHECK(inventory.itemAt(Cell{ 2, 0 }) == nullptr);
	CHECK(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 2, 1 }).outcome == Outcome::Done);
}

TEST_CASE("taking part of a stack splits it and leaves the rest in place")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "bandage", 5 }, Cell{ 1, 2 }).outcome == Outcome::Done);

	Taken taken = inventory.takeAt(Cell{ 1, 2 }, 2);

	CHECK(taken.outcome == Outcome::Done);
	CHECK(taken.item.type == "bandage");
	CHECK(taken.item.count == 2);
	REQUIRE(inventory.itemAt(Cell{ 1, 2 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 1, 2 })->count == 3);
}

TEST_CASE("taking from an empty cell, or a count that is not held, is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "bandage", 3 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	Cell cell{ 0, 0 };
	int count = 1;
	Outcome expected = Outcome::Done;

	SUBCASE("empty cell") { cell = Cell{ 3, 2 }; expected = Outcome::EmptyCell; }
	SUBCASE("outside the grid") { cell = Cell{ 9, 9 }; expected = Outcome::EmptyCell; }
	SUBCASE("zero") { count = 0; expected = Outcome::BadCount; }
	SUBCASE("more than held") { count = 4; expected = Outcome::BadCount; }

	Taken taken = inventory.takeAt(cell, count);

	CHECK(taken.outcome == expected);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 3);
}

TEST_CASE("the backpack lists its items top-left first, whatever order they arrived in")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 2 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 1, 2 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);

	std::vector<Placed> contents = inventory.backpackContents();

	REQUIRE(contents.size() == 3);
	CHECK(contents[0].anchor.column == 2);
	CHECK(contents[0].anchor.row == 0);
	CHECK(contents[1].anchor.column == 1);
	CHECK(contents[1].anchor.row == 2);
	CHECK(contents[2].anchor.column == 3);
	CHECK(contents[2].anchor.row == 2);
}

TEST_CASE("picking up tops up the top-left partial stack first")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "bandage", 4 }, Cell{ 3, 2 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "bandage", 4 }, Cell{ 0, 1 }).outcome == Outcome::Done);

	REQUIRE(inventory.pickUp(Item{ "bandage", 1 }).outcome == Outcome::Done);

	CHECK(inventory.itemAt(Cell{ 0, 1 })->count == 5);
	CHECK(inventory.itemAt(Cell{ 3, 2 })->count == 4);
}

TEST_CASE("moving an item drags it to a new anchor")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.move(Cell{ 1, 1 }, Cell{ 2, 1 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 1);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].anchor.column == 2);
	CHECK(inventory.backpackContents()[0].anchor.row == 1);
	CHECK(inventory.itemAt(Cell{ 0, 0 }) == nullptr);
}

TEST_CASE("a move that cannot land changes nothing")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 1 }).outcome == Outcome::Done);
	Cell to{ 0, 0 };
	Outcome expected = Outcome::Done;

	SUBCASE("onto another item") { to = Cell{ 2, 0 }; expected = Outcome::Overlaps; }
	SUBCASE("off the grid") { to = Cell{ 3, 2 }; expected = Outcome::OutOfBounds; }

	Result result = inventory.move(Cell{ 0, 0 }, to);

	CHECK(result.outcome == expected);
	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 2);
	CHECK(contents[0].item.type == "helmet");
	CHECK(contents[0].anchor.column == 0);
	CHECK(contents[0].anchor.row == 0);
}

TEST_CASE("an item can move onto part of its own old footprint")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 1, 4);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.move(Cell{ 0, 2 }, Cell{ 0, 1 });

	CHECK(result.outcome == Outcome::Done);
	CHECK(inventory.itemAt(Cell{ 0, 0 }) == nullptr);
	CHECK(inventory.itemAt(Cell{ 0, 3 }) != nullptr);
}

TEST_CASE("moving from an empty cell is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	Result result = inventory.move(Cell{ 1, 1 }, Cell{ 2, 2 });

	CHECK(result.outcome == Outcome::EmptyCell);
	CHECK(result.moved == 0);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("dragging a stack onto its own kind merges, and what does not fit stays put")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "bandage", 4 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "bandage", 3 }, Cell{ 2, 1 }).outcome == Outcome::Done);

	Result result = inventory.move(Cell{ 0, 0 }, Cell{ 2, 1 });

	CHECK(result.outcome == Outcome::Partial);
	CHECK(result.moved == 2);
	CHECK(result.leftover == 2);
	REQUIRE(inventory.itemAt(Cell{ 2, 1 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 2, 1 })->count == 5);
	REQUIRE(inventory.itemAt(Cell{ 0, 0 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->count == 2);
}

TEST_CASE("equipping moves an item from the backpack into a slot")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 1, 2 }, "main_hand");

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 1);
	REQUIRE(inventory.equippedIn("main_hand") != nullptr);
	CHECK(inventory.equippedIn("main_hand")->type == "pistol");
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("equipping refuses empty cells, unknown slots and slots the item is not allowed in")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	Cell cell{ 0, 0 };
	std::string slot = "main_hand";
	Outcome expected = Outcome::Done;

	SUBCASE("empty cell") { cell = Cell{ 3, 2 }; expected = Outcome::EmptyCell; }
	SUBCASE("unknown slot") { slot = "tail"; expected = Outcome::UnknownSlot; }
	SUBCASE("a pistol on the head") { slot = "head"; expected = Outcome::NotAllowed; }

	Result result = inventory.equipFrom(cell, slot);

	CHECK(result.outcome == expected);
	CHECK(result.moved == 0);
	CHECK(result.leftover == 0);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.equippedIn("main_hand") == nullptr);
	CHECK(inventory.equippedIn("head") == nullptr);
}

TEST_CASE("equipping into an occupied slot swaps, using the cells the new item left")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 2, 3);
	REQUIRE(inventory.placeAt(Item{ "armor", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "chest").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "vest", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 1, 1 }, "chest");

	CHECK(result.outcome == Outcome::Done);
	REQUIRE(inventory.equippedIn("chest") != nullptr);
	CHECK(inventory.equippedIn("chest")->type == "vest");
	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 1);
	CHECK(contents[0].item.type == "armor");
	CHECK(contents[0].anchor.column == 0);
	CHECK(contents[0].anchor.row == 0);
}

TEST_CASE("a swap whose old item has no room in the backpack changes nothing")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 2, 3);
	REQUIRE(inventory.placeAt(Item{ "armor", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "chest").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "vest", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 2 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 0, 0 }, "chest");

	CHECK(result.outcome == Outcome::NoRoom);
	CHECK(result.moved == 0);
	REQUIRE(inventory.equippedIn("chest") != nullptr);
	CHECK(inventory.equippedIn("chest")->type == "armor");
	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 2);
	CHECK(contents[0].item.type == "vest");
	CHECK(contents[1].item.type == "key");
}

TEST_CASE("equipping a two-handed weapon moves the off-hand item to the backpack and blocks the slot")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 3, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 3, 0 }, "off_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 0, 0 }, "main_hand");

	CHECK(result.outcome == Outcome::Done);
	REQUIRE(inventory.equippedIn("main_hand") != nullptr);
	CHECK(inventory.equippedIn("main_hand")->type == "shotgun");
	CHECK(inventory.equippedIn("off_hand") == nullptr);
	CHECK(inventory.isBlocked("off_hand"));
	CHECK_FALSE(inventory.isBlocked("main_hand"));
	std::vector<Placed> contents = inventory.backpackContents();
	REQUIRE(contents.size() == 1);
	CHECK(contents[0].item.type == "knife");
}

TEST_CASE("equipping into a blocked slot is refused and the blocking item stays")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 2, 0 }, "off_hand");

	CHECK(result.outcome == Outcome::Blocked);
	CHECK(result.moved == 0);
	CHECK(inventory.equippedIn("off_hand") == nullptr);
	REQUIRE(inventory.equippedIn("main_hand") != nullptr);
	CHECK(inventory.equippedIn("main_hand")->type == "shotgun");
	REQUIRE(inventory.itemAt(Cell{ 2, 0 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 2, 0 })->type == "knife");
}

TEST_CASE("a two-handed weapon displaces both hands, or nothing changes")
{
	Catalog catalog = fixture();

	SUBCASE("room for both")
	{
		Inventory inventory(catalog, 2, 4);
		REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
		REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
		REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
		REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "off_hand").outcome == Outcome::Done);
		REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 1, 0 }).outcome == Outcome::Done);

		CHECK(inventory.equipFrom(Cell{ 1, 0 }, "main_hand").outcome == Outcome::Done);
		std::vector<Placed> contents = inventory.backpackContents();
		REQUIRE(contents.size() == 2);
		CHECK(contents[0].item.type == "pistol");
		CHECK(contents[1].item.type == "knife");
	}
	SUBCASE("room for only one")
	{
		Inventory inventory(catalog, 1, 4);
		REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
		REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
		REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
		REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "off_hand").outcome == Outcome::Done);
		REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

		CHECK(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::NoRoom);
		CHECK(inventory.equippedIn("main_hand")->type == "pistol");
		CHECK(inventory.equippedIn("off_hand")->type == "knife");
		REQUIRE(inventory.backpackContents().size() == 1);
		CHECK(inventory.backpackContents()[0].item.type == "shotgun");
	}
}

TEST_CASE("unequipping moves the item into the backpack")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 2, 1 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 2, 1 }, "head").outcome == Outcome::Done);

	Result result = inventory.unequip("head");

	CHECK(result.outcome == Outcome::Done);
	CHECK(result.moved == 1);
	CHECK(inventory.equippedIn("head") == nullptr);
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].item.type == "helmet");
	CHECK(inventory.backpackContents()[0].anchor.column == 0);
	CHECK(inventory.backpackContents()[0].anchor.row == 0);
}

TEST_CASE("unequipping with no room in the backpack is refused and the item stays equipped")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 2, 2);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "head").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 1, 1 }).outcome == Outcome::Done);

	Result result = inventory.unequip("head");

	CHECK(result.outcome == Outcome::NoRoom);
	CHECK(result.moved == 0);
	CHECK(result.leftover == 0);
	REQUIRE(inventory.equippedIn("head") != nullptr);
	CHECK(inventory.equippedIn("head")->type == "helmet");
	CHECK(inventory.backpackContents().size() == 1);
}

TEST_CASE("unequipping an unknown or empty slot is refused")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);

	CHECK(inventory.unequip("tail").outcome == Outcome::UnknownSlot);
	CHECK(inventory.unequip("head").outcome == Outcome::EmptySlot);
	CHECK(inventory.backpackContents().empty());
}

TEST_CASE("unequipping a two-handed weapon frees the off hand")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 4);
	REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.unequip("main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 3, 0 }).outcome == Outcome::Done);

	CHECK_FALSE(inventory.isBlocked("off_hand"));
	CHECK(inventory.equipFrom(Cell{ 3, 0 }, "off_hand").outcome == Outcome::Done);
}

namespace
{
	struct Snapshot
	{
		std::vector<std::string> backpack;
		std::vector<std::string> equipped;

		bool operator==(const Snapshot& other) const { return backpack == other.backpack && equipped == other.equipped; }
	};

	const char* const slotIds[] = { "head", "chest", "main_hand", "off_hand" };
	const char* const typeIds[] = { "key", "pistol", "shotgun", "knife", "helmet", "armor", "vest", "bandage" };

	Snapshot snapshot(const Inventory& inventory)
	{
		Snapshot shot;
		for (const Placed& placed : inventory.backpackContents())
			shot.backpack.push_back(placed.item.type + "x" + std::to_string(placed.item.count) + "@"
				+ std::to_string(placed.anchor.column) + "," + std::to_string(placed.anchor.row));
		for (const char* slot : slotIds)
		{
			const Item* item = inventory.equippedIn(slot);
			shot.equipped.push_back(item ? item->type + "x" + std::to_string(item->count) : "-");
		}
		return shot;
	}

	void checkPromises(const Catalog& catalog, const Inventory& inventory, int columns, int rows,
		const std::map<std::string, int>& tally)
	{
		std::vector<int> cover(columns * rows, -1);
		std::map<std::string, int> held;
		std::vector<Placed> contents = inventory.backpackContents();
		for (int i = 0; i < (int)contents.size(); ++i)
		{
			const Placed& placed = contents[i];
			const ItemType* type = catalog.itemType(placed.item.type);
			REQUIRE(type != nullptr);
			REQUIRE(placed.anchor.column >= 0);
			REQUIRE(placed.anchor.row >= 0);
			REQUIRE(placed.anchor.column + type->width <= columns);
			REQUIRE(placed.anchor.row + type->height <= rows);
			REQUIRE(placed.item.count >= 1);
			REQUIRE(placed.item.count <= type->maxStack);
			for (int row = placed.anchor.row; row < placed.anchor.row + type->height; ++row)
				for (int column = placed.anchor.column; column < placed.anchor.column + type->width; ++column)
				{
					REQUIRE(cover[row * columns + column] == -1);
					cover[row * columns + column] = i;
				}
			held[placed.item.type] += placed.item.count;
		}
		for (int row = 0; row < rows; ++row)
			for (int column = 0; column < columns; ++column)
			{
				const Item* item = inventory.itemAt(Cell{ column, row });
				int covering = cover[row * columns + column];
				if (covering < 0)
					REQUIRE(item == nullptr);
				else
				{
					REQUIRE(item != nullptr);
					REQUIRE(item->type == contents[covering].item.type);
					REQUIRE(item->count == contents[covering].item.count);
				}
			}
		for (const char* slot : slotIds)
		{
			const Item* item = inventory.equippedIn(slot);
			if (inventory.isBlocked(slot))
				REQUIRE(item == nullptr);
			if (item)
			{
				REQUIRE(item->count == 1);
				const std::vector<std::string>& allowed = catalog.itemType(item->type)->slots;
				REQUIRE(std::find(allowed.begin(), allowed.end(), slot) != allowed.end());
				held[item->type] += 1;
			}
		}
		for (const char* type : typeIds)
		{
			CAPTURE(type);
			int expected = tally.count(type) ? tally.at(type) : 0;
			int actual = held.count(type) ? held.at(type) : 0;
			REQUIRE(actual == expected);
		}
	}
}

TEST_CASE("every promise holds across thousands of random operations")
{
	Catalog catalog = fixture();
	const int columns = 4;
	const int rows = 4;
	Inventory inventory(catalog, columns, rows);
	std::mt19937 random(20261010u);
	auto pick = [&](int below) { return (int)(random() % (std::uint32_t)below); };
	auto anyCell = [&]() { return Cell{ pick(columns + 2) - 1, pick(rows + 2) - 1 }; };
	auto anyItem = [&]()
	{
		int odd = pick(12);
		if (odd == 0)
			return Item{ "grenade", 1 };
		if (odd == 1)
			return Item{ typeIds[pick(8)], -pick(2) };
		if (odd == 2)
			return Item{ "key", 2 };
		std::string type = typeIds[pick(8)];
		return Item{ type, type == "bandage" ? 1 + pick(7) : 1 };
	};
	std::map<std::string, int> tally;
	int succeeded[6] = { 0, 0, 0, 0, 0, 0 };

	for (int step = 0; step < 3000; ++step)
	{
		CAPTURE(step);
		Snapshot before = snapshot(inventory);
		Outcome outcome = Outcome::Done;
		int operation = pick(6);
		if (operation == 0)
		{
			Item item = anyItem();
			Result result = inventory.pickUp(item);
			outcome = result.outcome;
			REQUIRE(result.moved + result.leftover == item.count);
			tally[item.type] += result.moved;
		}
		else if (operation == 1)
		{
			Item item = anyItem();
			Result result = inventory.placeAt(item, anyCell());
			outcome = result.outcome;
			REQUIRE(result.moved + result.leftover == item.count);
			tally[item.type] += result.moved;
		}
		else if (operation == 2)
		{
			std::vector<Placed> contents = inventory.backpackContents();
			Cell cell = !contents.empty() && pick(5) != 0 ? contents[pick((int)contents.size())].anchor : anyCell();
			const Item* item = inventory.itemAt(cell);
			int count = item ? 1 + pick(item->count) : 1;
			if (item && pick(10) == 0)
				count = pick(2) == 0 ? 0 : item->count + 1;
			Taken taken = inventory.takeAt(cell, count);
			outcome = taken.outcome;
			if (outcome == Outcome::Done)
				tally[taken.item.type] -= taken.item.count;
		}
		else if (operation == 3)
		{
			std::vector<Placed> contents = inventory.backpackContents();
			Cell from = !contents.empty() && pick(5) != 0 ? contents[pick((int)contents.size())].anchor : anyCell();
			outcome = inventory.move(from, anyCell()).outcome;
		}
		else if (operation == 4)
		{
			std::vector<Placed> contents = inventory.backpackContents();
			Cell cell = anyCell();
			std::string slot = pick(9) == 0 ? "tail" : slotIds[pick(4)];
			if (!contents.empty() && pick(5) != 0)
			{
				const Placed& chosen = contents[pick((int)contents.size())];
				cell = chosen.anchor;
				const std::vector<std::string>& allowed = catalog.itemType(chosen.item.type)->slots;
				if (!allowed.empty())
					slot = allowed[pick((int)allowed.size())];
			}
			outcome = inventory.equipFrom(cell, slot).outcome;
		}
		else
		{
			std::vector<std::string> worn;
			for (const char* slot : slotIds)
				if (inventory.equippedIn(slot))
					worn.push_back(slot);
			std::string slot = pick(9) == 0 ? "tail" : slotIds[pick(4)];
			if (!worn.empty() && pick(5) != 0)
				slot = worn[pick((int)worn.size())];
			outcome = inventory.unequip(slot).outcome;
		}

		if (outcome != Outcome::Done && outcome != Outcome::Partial)
			REQUIRE(snapshot(inventory) == before);
		else
			++succeeded[operation];
		checkPromises(catalog, inventory, columns, rows, tally);
	}
	for (int operation = 0; operation < 6; ++operation)
	{
		CAPTURE(operation);
		CHECK(succeeded[operation] >= 25);
	}
}

TEST_CASE("cells far outside the grid are out of bounds, not wrapped around")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	const int huge = std::numeric_limits<int>::max();

	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ huge, 0 }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, huge }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.backpackContents().empty());
}

static_assert(!std::is_constructible<Inventory, Catalog&&, int, int>::value,
	"an inventory must not be built from a temporary catalog it would outlive");

TEST_CASE("swapping out a two-handed weapon puts it in the freed cells and frees the off hand")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 1, 4);
	REQUIRE(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);

	Result result = inventory.equipFrom(Cell{ 0, 0 }, "main_hand");

	CHECK(result.outcome == Outcome::Done);
	CHECK(inventory.equippedIn("main_hand")->type == "pistol");
	REQUIRE(inventory.backpackContents().size() == 1);
	CHECK(inventory.backpackContents()[0].item.type == "shotgun");
	CHECK_FALSE(inventory.isBlocked("off_hand"));
}

static std::vector<std::string> occupancy(const Inventory& inventory, int columns, int rows)
{
	std::vector<std::string> lines;
	for (int row = 0; row < rows; ++row)
	{
		std::string line;
		for (int column = 0; column < columns; ++column)
		{
			const Item* item = inventory.itemAt(Cell{ column, row });
			line += item ? item->type[0] : '.';
		}
		lines.push_back(line);
	}
	return lines;
}

TEST_CASE("a rotated item lies on its side: its width and height swap where it is placed")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	Result placed = inventory.placeAt(Item{ "pistol", 1, true }, Cell{ 0, 1 });
	CHECK(placed.outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "....", "ppp.", "...." });
	REQUIRE(inventory.itemAt(Cell{ 2, 1 }) != nullptr);
	CHECK(inventory.itemAt(Cell{ 2, 1 })->rotated);
	CHECK(inventory.backpackContents()[0].item.rotated);
	CHECK(catalog.itemType("pistol")->width == 1);
	CHECK(catalog.itemType("pistol")->height == 3);
}

TEST_CASE("bounds and collisions use the rotated footprint")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	CHECK(inventory.placeAt(Item{ "shotgun", 1 }, Cell{ 0, 0 }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.placeAt(Item{ "shotgun", 1, true }, Cell{ 1, 0 }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.placeAt(Item{ "shotgun", 1, true }, Cell{ 0, 0 }).outcome == Outcome::Done);
	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 1 }).outcome == Outcome::Done);
	CHECK(inventory.placeAt(Item{ "pistol", 1, true }, Cell{ 1, 1 }).outcome == Outcome::Overlaps);
	CHECK(inventory.placeAt(Item{ "pistol", 1, true }, Cell{ 0, 1 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "ssss", "pppk", "...." });
	CHECK(inventory.placeAt(Item{ "armor", 1, true }, Cell{ 0, 2 }).outcome == Outcome::OutOfBounds);
}

TEST_CASE("rotating a placed item turns it where it lies when it still fits, and back again")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	Result turned = inventory.rotate(Cell{ 0, 2 });
	CHECK(turned.outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "ppp.", "....", "...." });
	CHECK(inventory.itemAt(Cell{ 0, 0 })->rotated);
	CHECK(inventory.rotate(Cell{ 2, 0 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "p...", "p...", "p..." });
	CHECK_FALSE(inventory.itemAt(Cell{ 0, 0 })->rotated);
}

TEST_CASE("a placed item that would hit a neighbour or the edge when turned stays as it was, with the reason")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 3, 1 }).outcome == Outcome::Done);
	std::vector<std::string> before = occupancy(inventory, 4, 3);
	CHECK(inventory.rotate(Cell{ 0, 1 }).outcome == Outcome::Overlaps);
	CHECK(inventory.rotate(Cell{ 3, 2 }).outcome == Outcome::OutOfBounds);
	CHECK(occupancy(inventory, 4, 3) == before);
	CHECK_FALSE(inventory.itemAt(Cell{ 0, 0 })->rotated);
	CHECK_FALSE(inventory.itemAt(Cell{ 3, 1 })->rotated);
	CHECK(inventory.rotate(Cell{ 1, 2 }).outcome == Outcome::EmptyCell);
	CHECK(inventory.backpackContents().size() == 3);
}

TEST_CASE("an item keeps its turn when taken out, placed again and moved")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "armor", 1, true }, Cell{ 0, 0 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "aaa.", "aaa.", "...." });
	Taken taken = inventory.takeAt(Cell{ 2, 1 }, 1);
	REQUIRE(taken.outcome == Outcome::Done);
	CHECK(taken.item.rotated);
	CHECK(inventory.placeAt(taken.item, Cell{ 1, 1 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "....", ".aaa", ".aaa" });
	CHECK(inventory.move(Cell{ 3, 2 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "aaa.", "aaa.", "...." });
	CHECK(inventory.move(Cell{ 0, 0 }, Cell{ 2, 0 }).outcome == Outcome::OutOfBounds);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "aaa.", "aaa.", "...." });
	REQUIRE(inventory.placeAt(Item{ "bandage", 4, true }, Cell{ 0, 2 }).outcome == Outcome::Done);
	Taken half = inventory.takeAt(Cell{ 0, 2 }, 2);
	REQUIRE(half.outcome == Outcome::Done);
	CHECK(half.item.rotated);
	CHECK(half.item.count == 2);
}

TEST_CASE("picking up finds room for an item the way it is turned")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	CHECK(inventory.pickUp(Item{ "shotgun", 1 }).outcome == Outcome::NoRoom);
	CHECK(inventory.pickUp(Item{ "shotgun", 1, true }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "ssss", "....", "...." });
	CHECK(inventory.pickUp(Item{ "pistol", 1, true }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "ssss", "ppp.", "...." });
	CHECK(inventory.pickUp(Item{ "bandage", 3, true }).outcome == Outcome::Done);
	CHECK(inventory.pickUp(Item{ "bandage", 2 }).outcome == Outcome::Done);
	CHECK(inventory.itemAt(Cell{ 3, 1 })->count == 5);
}

TEST_CASE("square items turn without complaint and without moving")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 0 }).outcome == Outcome::Done);
	std::vector<std::string> before = occupancy(inventory, 4, 3);
	CHECK(inventory.rotate(Cell{ 1, 1 }).outcome == Outcome::Done);
	CHECK(inventory.rotate(Cell{ 3, 0 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == before);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->rotated);
}

TEST_CASE("an item goes into an equipment slot upright and comes back out upright")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "knife", 1, true }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 1, 0 }, "main_hand").outcome == Outcome::Done);
	REQUIRE(inventory.equippedIn("main_hand") != nullptr);
	CHECK_FALSE(inventory.equippedIn("main_hand")->rotated);
	REQUIRE(inventory.unequip("main_hand").outcome == Outcome::Done);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "k...", "k...", "...." });
}

TEST_CASE("growing the grid keeps every item where it was and opens empty cells that take items")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 3, 2 }).outcome == Outcome::Done);
	CHECK(inventory.columns() == 4);
	CHECK(inventory.rows() == 3);
	CHECK(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 1, 3 }).outcome == Outcome::OutOfBounds);
	Result grown = inventory.resize(5, 5);
	CHECK(grown.outcome == Outcome::Done);
	CHECK(inventory.columns() == 5);
	CHECK(inventory.rows() == 5);
	CHECK(occupancy(inventory, 5, 5) == std::vector<std::string>{ "p....", "p....", "p..k.", ".....", "....." });
	CHECK(inventory.placeAt(Item{ "helmet", 1 }, Cell{ 1, 3 }).outcome == Outcome::Done);
	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ 4, 0 }).outcome == Outcome::Done);
	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ 4, 5 }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.rotate(Cell{ 2, 4 }).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 5, 5) == std::vector<std::string>{ "p...k", "p....", "p..k.", ".hh..", ".hh.." });
}

TEST_CASE("shrinking the grid works while everything still fits, and is refused, changing nothing, when an item would end up outside")
{
	Catalog catalog = fixture();
	Inventory inventory(catalog, 4, 5);
	REQUIRE(inventory.placeAt(Item{ "pistol", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "knife", 1 }, Cell{ 3, 2 }).outcome == Outcome::Done);
	CHECK(inventory.resize(4, 4).outcome == Outcome::Done);
	CHECK(inventory.rows() == 4);
	CHECK(inventory.resize(4, 3).outcome == Outcome::ItemsOutside);
	CHECK(inventory.rows() == 4);
	CHECK(occupancy(inventory, 4, 4) == std::vector<std::string>{ "p...", "p...", "p..k", "...k" });
	CHECK(inventory.resize(3, 4).outcome == Outcome::ItemsOutside);
	CHECK(inventory.columns() == 4);
	REQUIRE(inventory.move(Cell{ 3, 3 }, Cell{ 1, 0 }).outcome == Outcome::Done);
	CHECK(inventory.resize(2, 3).outcome == Outcome::Done);
	CHECK(occupancy(inventory, 2, 3) == std::vector<std::string>{ "pk", "pk", "p." });
	CHECK(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 0 }).outcome == Outcome::OutOfBounds);
	CHECK(inventory.resize(0, 3).outcome == Outcome::OutOfBounds);
	CHECK(inventory.resize(2, 0).outcome == Outcome::OutOfBounds);
	CHECK(inventory.columns() == 2);
	CHECK(inventory.rows() == 3);
}

static Catalog packs()
{
	Catalog catalog;
	LoadError error;
	bool loaded = loadCatalog(
		"id, name\n"
		"main_hand, Main Hand\n"
		"back, Back\n",
		"id, name, width, height, slots, blocks, max_stack, category, expand_cols, expand_rows\n"
		"key, Key, 1, 1, -, -, 1, key_item, 0, 0\n"
		"knife, Knife, 1, 2, main_hand, -, 1, weapon, 0, 0\n"
		"pack, Pack, 2, 2, back, -, 1, accessory, 0, 2\n"
		"bigpack, Big Pack, 2, 3, back, -, 1, accessory, 1, 4\n",
		catalog, error);
	INFO(error.file, " line ", error.line, ": ", error.message);
	REQUIRE(loaded);
	return catalog;
}

TEST_CASE("wearing a backpack grows the grid by its expansion, and taking it off puts the grid and the pack back")
{
	Catalog catalog = packs();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pack", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 1, 1 }, "back").outcome == Outcome::Done);
	CHECK(inventory.columns() == 4);
	CHECK(inventory.rows() == 5);
	CHECK(inventory.placeAt(Item{ "knife", 1 }, Cell{ 0, 3 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 4 }, "main_hand").outcome == Outcome::Done);
	CHECK(inventory.rows() == 5);
	REQUIRE(inventory.unequip("back").outcome == Outcome::Done);
	CHECK(inventory.rows() == 3);
	CHECK(inventory.equippedIn("back") == nullptr);
	CHECK(occupancy(inventory, 4, 3) == std::vector<std::string>{ "pp..", "pp..", "...." });
}

TEST_CASE("a backpack with something in its extra rows cannot be taken off until they are cleared")
{
	Catalog catalog = packs();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pack", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "back").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 2, 4 }).outcome == Outcome::Done);
	CHECK(inventory.unequip("back").outcome == Outcome::ItemsOutside);
	CHECK(inventory.equippedIn("back") != nullptr);
	CHECK(inventory.rows() == 5);
	CHECK(inventory.itemAt(Cell{ 2, 4 }) != nullptr);
	REQUIRE(inventory.move(Cell{ 2, 4 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	CHECK(inventory.unequip("back").outcome == Outcome::Done);
	CHECK(inventory.rows() == 3);
}

TEST_CASE("a backpack that would not fit back into the smaller grid stays on")
{
	Catalog catalog = packs();
	Inventory inventory(catalog, 2, 2);
	REQUIRE(inventory.placeAt(Item{ "pack", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "back").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 1, 1 }).outcome == Outcome::Done);
	CHECK(inventory.unequip("back").outcome == Outcome::NoRoom);
	CHECK(inventory.equippedIn("back") != nullptr);
	CHECK(inventory.rows() == 4);
	CHECK(inventory.itemAt(Cell{ 1, 1 }) != nullptr);
}

TEST_CASE("swapping to a bigger backpack grows the grid and drops the old one in; swapping down needs the extra rows clear")
{
	Catalog catalog = packs();
	Inventory inventory(catalog, 4, 3);
	REQUIRE(inventory.placeAt(Item{ "pack", 1 }, Cell{ 0, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 0, 0 }, "back").outcome == Outcome::Done);
	REQUIRE(inventory.placeAt(Item{ "bigpack", 1 }, Cell{ 2, 0 }).outcome == Outcome::Done);
	REQUIRE(inventory.equipFrom(Cell{ 2, 0 }, "back").outcome == Outcome::Done);
	CHECK(inventory.columns() == 5);
	CHECK(inventory.rows() == 7);
	CHECK(inventory.equippedIn("back")->type == "bigpack");
	CHECK(occupancy(inventory, 5, 7)[0] == "pp...");

	REQUIRE(inventory.placeAt(Item{ "key", 1 }, Cell{ 0, 6 }).outcome == Outcome::Done);
	CHECK(inventory.equipFrom(Cell{ 0, 0 }, "back").outcome == Outcome::ItemsOutside);
	CHECK(inventory.equippedIn("back")->type == "bigpack");
	CHECK(inventory.columns() == 5);
	CHECK(inventory.rows() == 7);
	CHECK(inventory.itemAt(Cell{ 0, 0 })->type == "pack");
	CHECK(inventory.itemAt(Cell{ 0, 6 }) != nullptr);

	REQUIRE(inventory.move(Cell{ 0, 6 }, Cell{ 3, 0 }).outcome == Outcome::Done);
	CHECK(inventory.equipFrom(Cell{ 0, 0 }, "back").outcome == Outcome::Done);
	CHECK(inventory.equippedIn("back")->type == "pack");
	CHECK(inventory.columns() == 4);
	CHECK(inventory.rows() == 5);
	bool bigpackBack = false;
	for (const Placed& placed : inventory.backpackContents())
		bigpackBack = bigpackBack || placed.item.type == "bigpack";
	CHECK(bigpackBack);
}
