#include "doctest.h"
#include "InventoryLayout.h"

using namespace inventory;

static void checkRect(const Rect& rect, int x, int y, int width, int height)
{
	CHECK(rect.x == x);
	CHECK(rect.y == y);
	CHECK(rect.width == width);
	CHECK(rect.height == height);
}

TEST_CASE("the first equipment slot is 180 by 44 at (28, 40)")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkRect(layout.slot(0), 28, 40, 180, 44);
}

TEST_CASE("equipment slots run four across, in two rows")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkRect(layout.slot(3), 592, 40, 180, 44);
	checkRect(layout.slot(4), 28, 92, 180, 44);
	checkRect(layout.slot(7), 592, 92, 180, 44);
}

TEST_CASE("backpack cells are 52 pixel squares centred below the slots")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkRect(layout.cell(Cell{ 0, 0 }), 192, 160, 52, 52);
	checkRect(layout.cell(Cell{ 7, 4 }), 556, 368, 52, 52);
}

TEST_CASE("slots and grid stay centred on a wider screen")
{
	InventoryLayout layout(1024, 768, 8, 8, 5);

	checkRect(layout.slot(0), 140, 40, 180, 44);
	checkRect(layout.cell(Cell{ 0, 0 }), 304, 160, 52, 52);
}

TEST_CASE("the grid starts 24 pixels below the last row of slots")
{
	InventoryLayout layout(800, 600, 4, 8, 5);

	CHECK(layout.cell(Cell{ 0, 0 }).y == 108);
}

TEST_CASE("an item's footprint spans the cells it covers")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkRect(layout.footprint(Cell{ 2, 1 }, 1, 3), 296, 212, 52, 156);
	checkRect(layout.footprint(Cell{ 0, 0 }, 2, 2), 192, 160, 104, 104);
}

static void checkCellHit(const Hit& hit, int column, int row)
{
	REQUIRE(hit.kind == Hit::Kind::Cell);
	CHECK(hit.cell.column == column);
	CHECK(hit.cell.row == row);
}

TEST_CASE("a point on the grid hits the cell under it, edges included")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkCellHit(layout.hitTest(358, 274), 3, 2);
	checkCellHit(layout.hitTest(192, 160), 0, 0);
	checkCellHit(layout.hitTest(607, 419), 7, 4);
	checkCellHit(layout.hitTest(243, 160), 0, 0);
	checkCellHit(layout.hitTest(244, 160), 1, 0);
	CHECK(layout.hitTest(191, 160).kind == Hit::Kind::None);
	CHECK(layout.hitTest(608, 200).kind == Hit::Kind::None);
	CHECK(layout.hitTest(300, 420).kind == Hit::Kind::None);
	CHECK(layout.hitTest(300, 159).kind == Hit::Kind::None);
}

static void checkSlotHit(const Hit& hit, int slot)
{
	REQUIRE(hit.kind == Hit::Kind::Slot);
	CHECK(hit.slot == slot);
}

TEST_CASE("a point on a slot hits that slot; the gaps between slots hit nothing")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkSlotHit(layout.hitTest(250, 100), 5);
	checkSlotHit(layout.hitTest(28, 40), 0);
	checkSlotHit(layout.hitTest(207, 83), 0);
	checkSlotHit(layout.hitTest(771, 135), 7);
	CHECK(layout.hitTest(772, 135).kind == Hit::Kind::None);
	CHECK(layout.hitTest(771, 136).kind == Hit::Kind::None);
	CHECK(layout.hitTest(208, 40).kind == Hit::Kind::None);
	CHECK(layout.hitTest(28, 84).kind == Hit::Kind::None);
	CHECK(layout.hitTest(27, 40).kind == Hit::Kind::None);
}

TEST_CASE("slots past the slot count are not hit")
{
	InventoryLayout layout(800, 600, 6, 8, 5);

	checkSlotHit(layout.hitTest(250, 100), 5);
	CHECK(layout.hitTest(450, 100).kind == Hit::Kind::None);
}

TEST_CASE("the panel fills the screen inside a 20 pixel margin")
{
	checkRect(InventoryLayout(800, 600, 8, 8, 5).panel(), 20, 20, 760, 560);
	checkRect(InventoryLayout(1024, 768, 8, 8, 5).panel(), 20, 20, 984, 728);
}

TEST_CASE("the status line sits 20 pixels below the grid, as wide as the slots")
{
	InventoryLayout layout(800, 600, 8, 8, 5);

	checkRect(layout.statusLine(), 28, 440, 744, 20);
}

TEST_CASE("a rectangle contains its top-left corner but not the pixel past its far edge")
{
	Rect rect{ 10, 20, 5, 4 };

	CHECK(rect.contains(10, 20));
	CHECK(rect.contains(14, 23));
	CHECK_FALSE(rect.contains(15, 20));
	CHECK_FALSE(rect.contains(10, 24));
	CHECK_FALSE(rect.contains(9, 20));
}

static bool inside(const Rect& inner, const Rect& outer)
{
	return inner.x >= outer.x && inner.y >= outer.y
		&& inner.x + inner.width <= outer.x + outer.width && inner.y + inner.height <= outer.y + outer.height;
}

TEST_CASE("slots, grid and status line all fit inside the panel at 800 by 600")
{
	InventoryLayout layout(800, 600, 8, 8, 5);
	Rect panel = layout.panel();

	for (int index = 0; index < 8; ++index)
	{
		CAPTURE(index);
		CHECK(inside(layout.slot(index), panel));
	}
	CHECK(inside(layout.footprint(Cell{ 0, 0 }, 8, 5), panel));
	CHECK(inside(layout.statusLine(), panel));
}
