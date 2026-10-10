#pragma once
#include "Inventory.h"
#include <vector>

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
		InventoryLayout(int screenWidth, int screenHeight, const std::vector<Cell>& slotPositions, int columns, int rows);

		Rect panel() const;
		Rect title() const;
		Rect dollArea() const;
		Rect details() const;
		Rect statusLine() const;
		Rect hintLine() const;
		Rect slot(int index) const;
		Rect cell(Cell position) const;
		Rect footprint(Cell anchor, int width, int height) const;
		Hit hitTest(int x, int y) const;

	private:
		int left_;
		int top_;
		std::vector<Cell> slotPositions_;
		int columns_;
		int rows_;
	};
}
