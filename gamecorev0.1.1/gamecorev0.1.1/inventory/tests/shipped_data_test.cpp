#include "doctest.h"
#include "Catalog.h"
#include <fstream>
#include <sstream>

using namespace inventory;

static std::string readDataFile(const std::string& name)
{
	std::string path = std::string(INVENTORY_DATA_DIR) + "/" + name;
	std::ifstream in(path, std::ios::binary);
	INFO("cannot open ", path);
	REQUIRE(in.is_open());
	std::ostringstream text;
	text << in.rdbuf();
	return text.str();
}

static Catalog shippedCatalog()
{
	Catalog catalog;
	LoadError error;
	bool loaded = loadCatalog(readDataFile("slots.csv"), readDataFile("items.csv"), catalog, error);
	INFO(error.file, " line ", error.line, ": ", error.message);
	REQUIRE(loaded);
	return catalog;
}

static const ItemType& shippedItem(const Catalog& catalog, const char* id)
{
	const ItemType* type = catalog.itemType(id);
	INFO("no item type '", id, "'");
	REQUIRE(type != nullptr);
	return *type;
}

TEST_CASE("the shipped slot and item files load")
{
	shippedCatalog();
}

TEST_CASE("the shipped slots are the owner's eight, in order, then the back for a backpack")
{
	Catalog catalog = shippedCatalog();

	std::vector<std::string> ids;
	for (int i = 0; i < catalog.slotCount(); ++i)
		ids.push_back(catalog.slotId(i));

	CHECK(ids == std::vector<std::string>{ "head", "chest", "gloves", "boots", "belt", "main_hand", "off_hand", "neck", "back" });
}

TEST_CASE("the shipped items have the owner's sizes")
{
	Catalog catalog = shippedCatalog();

	struct Size { const char* id; int width; int height; };
	const Size sizes[] = { { "key", 1, 1 }, { "pistol", 1, 3 }, { "body_armor", 2, 3 }, { "shotgun", 1, 4 } };
	for (const Size& size : sizes)
	{
		CAPTURE(size.id);
		const ItemType& type = shippedItem(catalog, size.id);
		CHECK(type.width == size.width);
		CHECK(type.height == size.height);
	}
}

TEST_CASE("only the shotgun is two-handed, blocking the off hand")
{
	Catalog catalog = shippedCatalog();

	const ItemType& shotgun = shippedItem(catalog, "shotgun");
	CHECK(shotgun.slots == std::vector<std::string>{ "main_hand" });
	CHECK(shotgun.blocks == std::vector<std::string>{ "off_hand" });

	for (const char* id : { "knife", "hatchet", "pistol", "flashlight", "lockpick", "key", "bandage", "pills",
		"ammo_box", "body_armor", "helmet", "gloves", "boots", "utility_belt", "amulet" })
	{
		CAPTURE(id);
		CHECK(shippedItem(catalog, id).blocks.empty());
	}
}

TEST_CASE("bandages, pills and ammo stack; lockpicks do not")
{
	Catalog catalog = shippedCatalog();

	for (const char* id : { "bandage", "pills", "ammo_box" })
	{
		CAPTURE(id);
		CHECK(shippedItem(catalog, id).maxStack > 1);
	}
	const ItemType& lockpick = shippedItem(catalog, "lockpick");
	CHECK(lockpick.maxStack == 1);
	CHECK(lockpick.width == 1);
	CHECK(lockpick.height == 1);
}

TEST_CASE("the utility belt equips to the belt slot")
{
	Catalog catalog = shippedCatalog();

	CHECK(shippedItem(catalog, "utility_belt").slots == std::vector<std::string>{ "belt" });
}

static bool iconExists(const std::string& name)
{
	std::ifstream in(std::string(INVENTORY_DATA_DIR) + "/../icons/" + name, std::ios::binary);
	return in.is_open();
}

TEST_CASE("every shipped item and slot names an icon file that exists")
{
	Catalog catalog = shippedCatalog();

	for (const char* id : { "knife", "hatchet", "pistol", "shotgun", "flashlight", "lockpick", "key", "bandage",
		"pills", "ammo_box", "body_armor", "helmet", "gloves", "boots", "utility_belt", "amulet", "small_backpack", "large_backpack" })
	{
		CAPTURE(id);
		const ItemType& type = shippedItem(catalog, id);
		REQUIRE_FALSE(type.icon.empty());
		CHECK(iconExists(type.icon));
	}
	for (int index = 0; index < catalog.slotCount(); ++index)
	{
		CAPTURE(catalog.slotId(index));
		REQUIRE_FALSE(catalog.slot(index).icon.empty());
		CHECK(iconExists(catalog.slot(index).icon));
	}
}

TEST_CASE("every shipped slot has its own place on the paper doll")
{
	Catalog catalog = shippedCatalog();

	for (int a = 0; a < catalog.slotCount(); ++a)
	{
		CAPTURE(catalog.slotId(a));
		CHECK(catalog.slot(a).dollColumn >= 0);
		CHECK(catalog.slot(a).dollRow >= 0);
		for (int b = a + 1; b < catalog.slotCount(); ++b)
			CHECK_FALSE((catalog.slot(a).dollColumn == catalog.slot(b).dollColumn
				&& catalog.slot(a).dollRow == catalog.slot(b).dollRow));
	}
}

TEST_CASE("the doll is seen from behind, so the main hand is on the right and the off hand on the left")
{
	Catalog catalog = shippedCatalog();
	int main = -1;
	int off = -1;
	for (int index = 0; index < catalog.slotCount(); ++index)
	{
		if (catalog.slotId(index) == "main_hand")
			main = catalog.slot(index).dollColumn;
		if (catalog.slotId(index) == "off_hand")
			off = catalog.slot(index).dollColumn;
	}
	CHECK(main == 2);
	CHECK(off == 0);
}

TEST_CASE("the small backpack is 2x2 and adds two rows, the large one is 2x3 and adds four, both worn on the back, and nothing else grows the grid")
{
	Catalog catalog = shippedCatalog();
	const ItemType& small = shippedItem(catalog, "small_backpack");
	const ItemType& large = shippedItem(catalog, "large_backpack");
	CHECK(small.name == "Small Backpack");
	CHECK(large.name == "Large Backpack");
	CHECK(small.width == 2);
	CHECK(small.height == 2);
	CHECK(large.width == 2);
	CHECK(large.height == 3);
	CHECK(small.slots == std::vector<std::string>{ "back" });
	CHECK(large.slots == std::vector<std::string>{ "back" });
	CHECK(small.expandRows == 2);
	CHECK(large.expandRows == 4);
	CHECK(small.expandColumns == 0);
	CHECK(large.expandColumns == 0);
	for (const char* id : { "knife", "hatchet", "pistol", "shotgun", "flashlight", "lockpick", "key", "bandage", "pills", "ammo_box",
		"body_armor", "helmet", "gloves", "boots", "utility_belt", "amulet" })
	{
		CAPTURE(id);
		CHECK(shippedItem(catalog, id).expandColumns == 0);
		CHECK(shippedItem(catalog, id).expandRows == 0);
	}
}
