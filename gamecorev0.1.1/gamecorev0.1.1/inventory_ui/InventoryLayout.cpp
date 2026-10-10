#include "InventoryLayout.h"

namespace inventory
{
	namespace
	{
		const int panelWidth = 760;
		const int panelHeight = 560;
		const int cellSize = 52;
	}

	InventoryLayout::InventoryLayout(int screenWidth, int screenHeight, const std::vector<Cell>& slotPositions,
		int columns, int rows)
		: left_((screenWidth - panelWidth) / 2), top_((screenHeight - panelHeight) / 2), slotPositions_(slotPositions),
		columns_(columns), rows_(rows)
	{
	}

	Rect InventoryLayout::panel() const
	{
		return Rect{ left_, top_, panelWidth, panelHeight };
	}

	Rect InventoryLayout::title() const
	{
		return Rect{ left_ + (panelWidth - 303) / 2, top_ + 8, 303, 72 };
	}

	Rect InventoryLayout::dollArea() const
	{
		return Rect{ left_ + 20, top_ + 92, 280, 420 };
	}

	Rect InventoryLayout::details() const
	{
		return Rect{ left_ + 320, top_ + 376, 416, 92 };
	}

	Rect InventoryLayout::statusLine() const
	{
		return Rect{ left_ + 320, top_ + 476, 416, 20 };
	}

	Rect InventoryLayout::hintLine() const
	{
		return Rect{ left_ + 20, top_ + 528, 720, 16 };
	}

	Rect InventoryLayout::slot(int index) const
	{
		Cell position = slotPositions_[index];
		if (position.column < 0 || position.row < 0)
			return Rect{ 0, 0, 0, 0 };
		Rect area = dollArea();
		return Rect{ area.x + 16 + position.column * 88, area.y + 16 + position.row * 100, 72, 72 };
	}

	Rect InventoryLayout::cell(Cell position) const
	{
		return Rect{ left_ + 320 + position.column * cellSize, top_ + 100 + position.row * cellSize, cellSize, cellSize };
	}

	Rect InventoryLayout::footprint(Cell anchor, int width, int height) const
	{
		Rect corner = cell(anchor);
		return Rect{ corner.x, corner.y, width * cellSize, height * cellSize };
	}

	Hit InventoryLayout::hitTest(int x, int y) const
	{
		Rect grid = footprint(Cell{ 0, 0 }, columns_, rows_);
		if (grid.contains(x, y))
			return Hit{ Hit::Kind::Cell, Cell{ (x - grid.x) / cellSize, (y - grid.y) / cellSize }, -1 };
		for (int index = 0; index < (int)slotPositions_.size(); ++index)
			if (slot(index).contains(x, y))
				return Hit{ Hit::Kind::Slot, Cell{ 0, 0 }, index };
		return Hit{ Hit::Kind::None, Cell{ 0, 0 }, -1 };
	}
}
