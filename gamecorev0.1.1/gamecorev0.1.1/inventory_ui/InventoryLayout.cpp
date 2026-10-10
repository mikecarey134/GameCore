#include "InventoryLayout.h"

namespace inventory
{
	namespace
	{
		const int slotWidth = 180;
		const int slotHeight = 44;
		const int gap = 8;
		const int slotsPerRow = 4;
		const int cellSize = 52;
		const int margin = 20;
		const int slotsTop = 40;
		const int gridGap = 24;
		const int statusGap = 20;
		const int statusHeight = 20;
		const int slotBlockWidth = slotsPerRow * slotWidth + (slotsPerRow - 1) * gap;
	}

	InventoryLayout::InventoryLayout(int screenWidth, int screenHeight, int slotCount, int columns, int rows)
		: screenWidth_(screenWidth), screenHeight_(screenHeight), slotCount_(slotCount), columns_(columns), rows_(rows)
	{
	}

	int InventoryLayout::slotBlockLeft() const
	{
		return (screenWidth_ - slotBlockWidth) / 2;
	}

	int InventoryLayout::gridTop() const
	{
		int slotRows = (slotCount_ + slotsPerRow - 1) / slotsPerRow;
		return slotsTop + slotRows * (slotHeight + gap) - gap + gridGap;
	}

	Rect InventoryLayout::slot(int index) const
	{
		return Rect{ slotBlockLeft() + (index % slotsPerRow) * (slotWidth + gap), slotsTop + (index / slotsPerRow) * (slotHeight + gap),
			slotWidth, slotHeight };
	}

	Rect InventoryLayout::cell(Cell position) const
	{
		int left = (screenWidth_ - columns_ * cellSize) / 2;
		return Rect{ left + position.column * cellSize, gridTop() + position.row * cellSize, cellSize, cellSize };
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
		for (int index = 0; index < slotCount_; ++index)
			if (slot(index).contains(x, y))
				return Hit{ Hit::Kind::Slot, Cell{ 0, 0 }, index };
		return Hit{ Hit::Kind::None, Cell{ 0, 0 }, -1 };
	}

	Rect InventoryLayout::panel() const
	{
		return Rect{ margin, margin, screenWidth_ - 2 * margin, screenHeight_ - 2 * margin };
	}

	Rect InventoryLayout::statusLine() const
	{
		return Rect{ slotBlockLeft(), gridTop() + rows_ * cellSize + statusGap, slotBlockWidth, statusHeight };
	}
}
