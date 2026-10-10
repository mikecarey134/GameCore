#include "doctest.h"
#include "Catalog.h"

using namespace inventory;

static const char* const oneSlot =
	"id, name\n"
	"head, Head\n";

static const char* const itemsHeader =
	"id, name, width, height, slots, blocks, max_stack, category\n";

TEST_CASE("a valid catalog loads its slots and item types")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) +
		"helmet, Helmet, 2, 3, head, -, 1, armor\n";

	REQUIRE(loadCatalog(oneSlot, items, catalog, error));

	CHECK(catalog.slotCount() == 1);
	CHECK(catalog.slotId(0) == "head");
	CHECK(catalog.slotName(0) == "Head");
	const ItemType* helmet = catalog.itemType("helmet");
	REQUIRE(helmet != nullptr);
	CHECK(helmet->name == "Helmet");
	CHECK(helmet->width == 2);
	CHECK(helmet->height == 3);
	CHECK(helmet->slots == std::vector<std::string>{"head"});
	CHECK(helmet->blocks.empty());
	CHECK(helmet->maxStack == 1);
	CHECK(helmet->category == Category::Armor);
}

TEST_CASE("a missing column is reported with its file and the header line")
{
	Catalog catalog;
	LoadError error;
	std::string items =
		"\n"
		"id, name, width, height, slots, max_stack, category\n"
		"helmet, Helmet, 2, 3, head, 1, armor\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.file == "items.csv");
	CHECK(error.line == 2);
	CHECK(error.message == "missing column 'blocks'");
}

TEST_CASE("a missing slot column is reported against slots.csv")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "key, Key, 1, 1, -, -, 1, key_item\n";

	CHECK_FALSE(loadCatalog("id\nhead\n", items, catalog, error));
	CHECK(error.file == "slots.csv");
	CHECK(error.line == 1);
	CHECK(error.message == "missing column 'name'");
}

TEST_CASE("a row with the wrong number of cells is reported by its line")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) +
		"key, Key, 1, 1, -, -, 1, key_item\n"
		"pistol, Pistol, 1, 3, -, 1, weapon\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.file == "items.csv");
	CHECK(error.line == 3);
	CHECK(error.message == "7 cells, but the header has 8");
}

TEST_CASE("a duplicate item id is reported by its line")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) +
		"key, Key, 1, 1, -, -, 1, key_item\n"
		"key, Old Key, 1, 1, -, -, 1, key_item\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 3);
	CHECK(error.message == "duplicate item id 'key'");
}

TEST_CASE("sizes and stack limits must be whole numbers of at least 1")
{
	Catalog catalog;
	LoadError error;
	std::string row;
	std::string expected;

	SUBCASE("zero width") { row = "key, Key, 0, 1, -, -, 1, key_item"; expected = "width must be a whole number of at least 1"; }
	SUBCASE("worded height") { row = "key, Key, 1, tall, -, -, 1, key_item"; expected = "height must be a whole number of at least 1"; }
	SUBCASE("negative stack") { row = "key, Key, 1, 1, -, -, -2, key_item"; expected = "max_stack must be a whole number of at least 1"; }

	CHECK_FALSE(loadCatalog(oneSlot, std::string(itemsHeader) + row + "\n", catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == expected);
}

TEST_CASE("an unknown category is rejected")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "key, Key, 1, 1, -, -, 1, keyitem\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "unknown category 'keyitem'");
}

TEST_CASE("only consumables may stack")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) +
		"bandage, Bandage, 1, 1, -, -, 5, consumable\n"
		"lockpick, Lockpick, 1, 1, -, -, 5, tool\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 3);
	CHECK(error.message == "'lockpick' stacks but is not a consumable");
}

TEST_CASE("an item may only name slots that slots.csv defines")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "ring, Ring, 1, 1, finger, -, 1, accessory\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "unknown slot 'finger'");
}

TEST_CASE("an item may only block slots that slots.csv defines")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "helmet, Helmet, 2, 2, head, face, 1, armor\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "unknown slot 'face'");
}

TEST_CASE("an item that cannot be equipped cannot block slots")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "rock, Rock, 1, 1, -, head, 1, tool\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "'rock' blocks slots but cannot be equipped");
}

TEST_CASE("an item that stacks cannot be equipped")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "bandage, Bandage, 1, 1, head, -, 5, consumable\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "'bandage' stacks, so it cannot be equipped");
}

TEST_CASE("an empty slot list must be written as '-'")
{
	Catalog catalog;
	LoadError error;
	std::string row;
	std::string expected;

	SUBCASE("slots") { row = "key, Key, 1, 1, , -, 1, key_item"; expected = "slots is empty; write '-' for none"; }
	SUBCASE("blocks") { row = "key, Key, 1, 1, -, , 1, key_item"; expected = "blocks is empty; write '-' for none"; }

	CHECK_FALSE(loadCatalog(oneSlot, std::string(itemsHeader) + row + "\n", catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == expected);
}

TEST_CASE("an item id may not be empty")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + ", Key, 1, 1, -, -, 1, key_item\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "empty item id");
}

TEST_CASE("slot rows need one unique, non-empty id and every cell")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) + "key, Key, 1, 1, -, -, 1, key_item\n";
	std::string slots;
	std::string expected;

	SUBCASE("cell count") { slots = "id, name\nhead, Head\nneck\n"; expected = "1 cells, but the header has 2"; }
	SUBCASE("empty id") { slots = "id, name\nhead, Head\n, Neck\n"; expected = "empty slot id"; }
	SUBCASE("duplicate id") { slots = "id, name\nhead, Head\nhead, Hat\n"; expected = "duplicate slot id 'head'"; }

	CHECK_FALSE(loadCatalog(slots, items, catalog, error));
	CHECK(error.file == "slots.csv");
	CHECK(error.line == 3);
	CHECK(error.message == expected);
}

TEST_CASE("a file with no rows is rejected as a whole")
{
	Catalog catalog;
	LoadError error;
	std::string slots = oneSlot;
	std::string items = std::string(itemsHeader) + "key, Key, 1, 1, -, -, 1, key_item\n";
	std::string file;
	std::string expected;

	SUBCASE("slots") { slots = "id, name\n"; file = "slots.csv"; expected = "no slots"; }
	SUBCASE("items") { items = itemsHeader; file = "items.csv"; expected = "no item types"; }

	CHECK_FALSE(loadCatalog(slots, items, catalog, error));
	CHECK(error.file == file);
	CHECK(error.line == 0);
	CHECK(error.message == expected);
}

TEST_CASE("a failed load leaves the catalog empty, even one loaded before")
{
	Catalog catalog;
	LoadError error;
	std::string good = std::string(itemsHeader) + "key, Key, 1, 1, -, -, 1, key_item\n";
	std::string bad = std::string(itemsHeader) + "ring, Ring, 1, 1, finger, -, 1, accessory\n";
	REQUIRE(loadCatalog(oneSlot, good, catalog, error));

	CHECK_FALSE(loadCatalog(oneSlot, bad, catalog, error));
	CHECK(catalog.slotCount() == 0);
	CHECK(catalog.itemType("key") == nullptr);
}

TEST_CASE("files saved by a Windows editor load")
{
	Catalog catalog;
	LoadError error;
	std::string bom = "\xEF\xBB\xBF";
	std::string slots = bom + "id, name\r\nhead, Head\r\n";
	std::string items = bom + "id, name, width, height, slots, blocks, max_stack, category\r\n"
		"helmet, Helmet, 2, 2, head, -, 1, armor\r\n";

	REQUIRE(loadCatalog(slots, items, catalog, error));
	CHECK(catalog.slotId(0) == "head");
	REQUIRE(catalog.itemType("helmet") != nullptr);
	CHECK(catalog.itemType("helmet")->category == Category::Armor);
}

TEST_CASE("columns are found by name, in any order")
{
	Catalog catalog;
	LoadError error;
	std::string slots = "name, id\nHead, head\n";
	std::string items =
		"category, max_stack, blocks, slots, height, width, name, id\n"
		"armor, 1, -, head, 3, 2, Helmet, helmet\n";

	REQUIRE(loadCatalog(slots, items, catalog, error));
	CHECK(catalog.slotId(0) == "head");
	CHECK(catalog.slotName(0) == "Head");
	REQUIRE(catalog.itemType("helmet") != nullptr);
	CHECK(catalog.itemType("helmet")->width == 2);
	CHECK(catalog.itemType("helmet")->height == 3);
}

TEST_CASE("slot lists separate several slots with '|'")
{
	Catalog catalog;
	LoadError error;
	std::string slots = "id, name\nmain_hand, Main Hand\noff_hand, Off Hand\nhead, Head\n";
	std::string items = std::string(itemsHeader) +
		"knife, Knife, 1, 2, main_hand|off_hand, -, 1, weapon\n"
		"great_helm, Great Helm, 2, 2, head, main_hand|off_hand, 1, armor\n";

	REQUIRE(loadCatalog(slots, items, catalog, error));
	REQUIRE(catalog.itemType("knife") != nullptr);
	CHECK(catalog.itemType("knife")->slots == std::vector<std::string>{"main_hand", "off_hand"});
	REQUIRE(catalog.itemType("great_helm") != nullptr);
	CHECK(catalog.itemType("great_helm")->blocks == std::vector<std::string>{"main_hand", "off_hand"});
	CHECK(catalog.hasSlot("off_hand"));
	CHECK_FALSE(catalog.hasSlot("finger"));
}

TEST_CASE("a column named twice is rejected")
{
	Catalog catalog;
	LoadError error;
	std::string items =
		"id, name, width, height, slots, blocks, max_stack, category, width\n"
		"key, Key, 1, 1, -, -, 1, key_item, 0\n";

	CHECK_FALSE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(error.file == "items.csv");
	CHECK(error.line == 1);
	CHECK(error.message == "column 'width' appears twice");
}

TEST_CASE("blank and whitespace-only lines are skipped anywhere")
{
	Catalog catalog;
	LoadError error;
	std::string items = std::string(itemsHeader) +
		"key, Key, 1, 1, -, -, 1, key_item\r\n"
		"\r\n"
		"   \t\n"
		"pistol, Pistol, 1, 3, head, -, 1, weapon\n"
		"\n";

	REQUIRE(loadCatalog(oneSlot, items, catalog, error));
	CHECK(catalog.itemType("key") != nullptr);
	CHECK(catalog.itemType("pistol") != nullptr);
}

TEST_CASE("an item may not block a slot it can be equipped in")
{
	Catalog catalog;
	LoadError error;
	std::string slots = "id, name\nmain_hand, Main Hand\noff_hand, Off Hand\n";
	std::string items = std::string(itemsHeader) +
		"buckler, Buckler, 1, 2, main_hand|off_hand, off_hand, 1, armor\n";

	CHECK_FALSE(loadCatalog(slots, items, catalog, error));
	CHECK(error.line == 2);
	CHECK(error.message == "'buckler' blocks off_hand, a slot it can be equipped in");
}
