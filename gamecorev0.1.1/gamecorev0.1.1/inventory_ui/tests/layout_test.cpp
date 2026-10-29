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

static const std::vector<Cell> doll = {
	Cell{ 1, 0 }, Cell{ 1, 1 }, Cell{ 0, 1 }, Cell{ 1, 3 },
	Cell{ 1, 2 }, Cell{ 0, 2 }, Cell{ 2, 2 }, Cell{ 2, 0 } };

TEST_CASE("the panel is a 760 by 560 block centred on the screen")
{
	checkRect(InventoryLayout(800, 600, doll, 8, 5).panel(), 20, 20, 760, 560);
	checkRect(InventoryLayout(1024, 768, doll, 8, 5).panel(), 132, 104, 760, 560);
}

TEST_CASE("the title, doll area, details box, status line and hint line sit at fixed places in the panel")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkRect(layout.title(), 248, 28, 303, 72);
	checkRect(layout.dollArea(), 40, 112, 280, 420);
	checkRect(layout.details(), 340, 396, 416, 92);
	checkRect(layout.statusLine(), 340, 496, 416, 20);
	checkRect(layout.hintLine(), 40, 548, 720, 16);
}

TEST_CASE("each slot is a 72 pixel box at its paper-doll position")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkRect(layout.slot(0), 144, 128, 72, 72);
	checkRect(layout.slot(5), 56, 328, 72, 72);
	checkRect(layout.slot(3), 144, 428, 72, 72);
	checkRect(layout.slot(7), 232, 128, 72, 72);
}

TEST_CASE("backpack cells are 52 pixel squares on the right of the panel")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkRect(layout.cell(Cell{ 0, 0 }), 340, 120, 52, 52);
	checkRect(layout.cell(Cell{ 7, 4 }), 704, 328, 52, 52);
}

TEST_CASE("an item's footprint spans the cells it covers")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkRect(layout.footprint(Cell{ 2, 1 }, 1, 3), 444, 172, 52, 156);
	checkRect(layout.footprint(Cell{ 0, 0 }, 2, 2), 340, 120, 104, 104);
}

static void checkCellHit(const Hit& hit, int column, int row)
{
	REQUIRE(hit.kind == Hit::Kind::Cell);
	CHECK(hit.cell.column == column);
	CHECK(hit.cell.row == row);
}

static void checkSlotHit(const Hit& hit, int slot)
{
	REQUIRE(hit.kind == Hit::Kind::Slot);
	CHECK(hit.slot == slot);
}

TEST_CASE("a point on the grid hits the cell under it, edges included")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkCellHit(layout.hitTest(340, 120), 0, 0);
	checkCellHit(layout.hitTest(391, 120), 0, 0);
	checkCellHit(layout.hitTest(392, 120), 1, 0);
	checkCellHit(layout.hitTest(755, 379), 7, 4);
	CHECK(layout.hitTest(339, 120).kind == Hit::Kind::None);
	CHECK(layout.hitTest(756, 120).kind == Hit::Kind::None);
	CHECK(layout.hitTest(500, 380).kind == Hit::Kind::None);
	CHECK(layout.hitTest(500, 119).kind == Hit::Kind::None);
}

TEST_CASE("a point on a slot hits that slot; the gaps between slots hit nothing")
{
	InventoryLayout layout(800, 600, doll, 8, 5);

	checkSlotHit(layout.hitTest(144, 128), 0);
	checkSlotHit(layout.hitTest(215, 199), 0);
	checkSlotHit(layout.hitTest(60, 330), 5);
	CHECK(layout.hitTest(216, 150).kind == Hit::Kind::None);
	CHECK(layout.hitTest(150, 200).kind == Hit::Kind::None);
	CHECK(layout.hitTest(60, 150).kind == Hit::Kind::None);
}

TEST_CASE("a slot without a doll position has no box and is never hit")
{
	InventoryLayout layout(800, 600, std::vector<Cell>{ Cell{ 1, 0 }, Cell{ -1, -1 } }, 8, 5);

	checkRect(layout.slot(1), 0, 0, 0, 0);
	CHECK(layout.hitTest(10, 50).kind == Hit::Kind::None);
	CHECK(layout.hitTest(0, 0).kind == Hit::Kind::None);
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

TEST_CASE("everything fits inside the panel, and the slots inside the doll area")
{
	InventoryLayout layout(800, 600, doll, 8, 5);
	Rect panel = layout.panel();

	CHECK(inside(layout.title(), panel));
	CHECK(inside(layout.dollArea(), panel));
	CHECK(inside(layout.footprint(Cell{ 0, 0 }, 8, 5), panel));
	CHECK(inside(layout.details(), panel));
	CHECK(inside(layout.statusLine(), panel));
	CHECK(inside(layout.hintLine(), panel));
	for (int index = 0; index < 8; ++index)
	{
		CAPTURE(index);
		CHECK(inside(layout.slot(index), layout.dollArea()));
	}
}
