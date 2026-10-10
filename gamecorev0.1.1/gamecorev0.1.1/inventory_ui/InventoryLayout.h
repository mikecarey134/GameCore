#pragma once
#include "Inventory.h"

namespace inventory
{
	struct Rect
	{
		int x;
		int y;
		int width;
		int height;

		bool contains(int px, int py) const { return px >= x && py >= y && px < x + width && py < y + height; }
	};

	struct Hit
	{
		enum class Kind { None, Cell, Slot };
		Kind kind;
		Cell cell;
		int slot;
	};

	class InventoryLayout
	{
	public:
		InventoryLayout(int screenWidth, int screenHeight, int slotCount, int columns, int rows);

		Rect panel() const;
		Rect slot(int index) const;
		Rect cell(Cell position) const;
		Rect footprint(Cell anchor, int width, int height) const;
		Rect statusLine() const;
		Hit hitTest(int x, int y) const;

	private:
		int slotBlockLeft() const;
		int gridTop() const;

		int screenWidth_;
		int screenHeight_;
		int slotCount_;
		int columns_;
		int rows_;
	};
}
