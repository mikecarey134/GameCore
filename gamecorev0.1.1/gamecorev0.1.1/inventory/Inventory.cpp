#include "Inventory.h"
#include <algorithm>

namespace inventory
{
	namespace
	{
		Result finished(int moved, int leftover)
		{
			if (leftover == 0)
				return Result{ Outcome::Done, moved, 0 };
			return Result{ moved > 0 ? Outcome::Partial : Outcome::NoRoom, moved, leftover };
		}

		int across(const ItemType& type, bool rotated)
		{
			return rotated ? type.height : type.width;
		}

		int down(const ItemType& type, bool rotated)
		{
			return rotated ? type.width : type.height;
		}

		bool contains(const std::vector<std::string>& ids, const std::string& id)
		{
			return std::find(ids.begin(), ids.end(), id) != ids.end();
		}
	}

	Inventory::Inventory(const Catalog& catalog, int columns, int rows)
		: catalog_(catalog), baseColumns_(columns), baseRows_(rows), columns_(columns), rows_(rows),
		occupant_(columns * rows, -1), equipped_(catalog.slotCount(), Item{})
	{
	}

	int Inventory::slotIndex(const std::string& slotId) const
	{
		for (int i = 0; i < catalog_.slotCount(); ++i)
			if (catalog_.slotId(i) == slotId)
				return i;
		return -1;
	}

	int Inventory::placementAt(Cell cell) const
	{
		if (cell.column < 0 || cell.row < 0 || cell.column >= columns_ || cell.row >= rows_)
			return -1;
		return occupant_[cell.row * columns_ + cell.column];
	}

	void Inventory::reindex()
	{
		std::fill(occupant_.begin(), occupant_.end(), -1);
		for (int i = 0; i < (int)placements_.size(); ++i)
		{
			const Placed& placed = placements_[i];
			const ItemType* type = catalog_.itemType(placed.item.type);
			for (int row = placed.anchor.row; row < placed.anchor.row + down(*type, placed.item.rotated); ++row)
				for (int column = placed.anchor.column; column < placed.anchor.column + across(*type, placed.item.rotated); ++column)
					occupant_[row * columns_ + column] = i;
		}
	}

	void Inventory::insertInOrder(const Placed& placed)
	{
		auto later = std::find_if(placements_.begin(), placements_.end(), [&](const Placed& other)
		{
			return other.anchor.row != placed.anchor.row ? other.anchor.row > placed.anchor.row
				: other.anchor.column > placed.anchor.column;
		});
		placements_.insert(later, placed);
		reindex();
	}

	void Inventory::removeAt(int index)
	{
		placements_.erase(placements_.begin() + index);
		reindex();
	}

	Outcome Inventory::validate(const Item& item, const ItemType*& type) const
	{
		type = catalog_.itemType(item.type);
		if (!type)
			return Outcome::UnknownItem;
		if (item.count < 1 || (type->maxStack == 1 && item.count != 1))
			return Outcome::BadCount;
		return Outcome::Done;
	}

	bool Inventory::inBounds(const ItemType& type, bool rotated, Cell anchor) const
	{
		return anchor.column >= 0 && anchor.row >= 0
			&& anchor.column <= columns_ - across(type, rotated) && anchor.row <= rows_ - down(type, rotated);
	}

	bool Inventory::fits(const ItemType& type, bool rotated, Cell anchor) const
	{
		if (!inBounds(type, rotated, anchor))
			return false;
		for (int row = anchor.row; row < anchor.row + down(type, rotated); ++row)
			for (int column = anchor.column; column < anchor.column + across(type, rotated); ++column)
				if (occupant_[row * columns_ + column] >= 0)
					return false;
		return true;
	}

	Result Inventory::pickUp(const Item& item)
	{
		const ItemType* type = nullptr;
		Outcome problem = validate(item, type);
		if (problem != Outcome::Done)
			return Result{ problem, 0, item.count };

		int remaining = item.count;
		for (Placed& placed : placements_)
			if (placed.item.type == item.type && remaining > 0)
			{
				int added = std::min(remaining, type->maxStack - placed.item.count);
				placed.item.count += added;
				remaining -= added;
			}

		for (int row = 0; row < rows_ && remaining > 0; ++row)
			for (int column = 0; column < columns_ && remaining > 0; ++column)
				if (fits(*type, item.rotated, Cell{ column, row }))
				{
					int stack = std::min(remaining, type->maxStack);
					insertInOrder(Placed{ Item{ item.type, stack, item.rotated }, Cell{ column, row } });
					remaining -= stack;
				}
		return finished(item.count - remaining, remaining);
	}

	Result Inventory::placeAt(const Item& item, Cell anchor)
	{
		const ItemType* type = nullptr;
		Outcome problem = validate(item, type);
		if (problem != Outcome::Done)
			return Result{ problem, 0, item.count };

		int existing = placementAt(anchor);
		if (existing >= 0 && placements_[existing].item.type == item.type && type->maxStack > 1)
		{
			Item& stack = placements_[existing].item;
			int added = std::min(item.count, type->maxStack - stack.count);
			stack.count += added;
			return finished(added, item.count - added);
		}

		if (!inBounds(*type, item.rotated, anchor))
			return Result{ Outcome::OutOfBounds, 0, item.count };
		if (!fits(*type, item.rotated, anchor))
			return Result{ Outcome::Overlaps, 0, item.count };
		int placed = std::min(item.count, type->maxStack);
		insertInOrder(Placed{ Item{ item.type, placed, item.rotated }, anchor });
		return finished(placed, item.count - placed);
	}

	Taken Inventory::takeAt(Cell cell, int count)
	{
		int index = placementAt(cell);
		if (index < 0)
			return Taken{ Outcome::EmptyCell, Item{} };
		Item& held = placements_[index].item;
		if (count < 1 || count > held.count)
			return Taken{ Outcome::BadCount, Item{} };
		if (count < held.count)
		{
			held.count -= count;
			return Taken{ Outcome::Done, Item{ held.type, count, held.rotated } };
		}
		Item taken = held;
		removeAt(index);
		return Taken{ Outcome::Done, taken };
	}

	Result Inventory::move(Cell from, Cell toAnchor)
	{
		int index = placementAt(from);
		if (index < 0)
			return Result{ Outcome::EmptyCell, 0, 0 };
		Placed original = placements_[index];
		removeAt(index);
		Result result = placeAt(original.item, toAnchor);
		if (result.leftover > 0)
			insertInOrder(Placed{ Item{ original.item.type, result.leftover, original.item.rotated }, original.anchor });
		return result;
	}

	Result Inventory::rotate(Cell cell)
	{
		int index = placementAt(cell);
		if (index < 0)
			return Result{ Outcome::EmptyCell, 0, 0 };
		Placed original = placements_[index];
		const ItemType* type = catalog_.itemType(original.item.type);
		removeAt(index);
		bool turned = !original.item.rotated;
		Outcome outcome = !inBounds(*type, turned, original.anchor) ? Outcome::OutOfBounds
			: !fits(*type, turned, original.anchor) ? Outcome::Overlaps : Outcome::Done;
		if (outcome == Outcome::Done)
			original.item.rotated = turned;
		insertInOrder(original);
		return Result{ outcome, outcome == Outcome::Done ? 1 : 0, 0 };
	}

	void Inventory::restore(const std::vector<Placed>& placements, const std::vector<Item>& equipped, int columns, int rows)
	{
		placements_ = placements;
		equipped_ = equipped;
		columns_ = columns;
		rows_ = rows;
		occupant_.assign(columns * rows, -1);
		reindex();
	}

	Result Inventory::resize(int columns, int rows)
	{
		if (columns < 1 || rows < 1)
			return Result{ Outcome::OutOfBounds, 0, 0 };
		for (const Placed& placed : placements_)
		{
			const ItemType* type = catalog_.itemType(placed.item.type);
			if (placed.anchor.column + across(*type, placed.item.rotated) > columns || placed.anchor.row + down(*type, placed.item.rotated) > rows)
				return Result{ Outcome::ItemsOutside, 0, 0 };
		}
		columns_ = columns;
		rows_ = rows;
		occupant_.assign(columns * rows, -1);
		reindex();
		return Result{ Outcome::Done, 0, 0 };
	}

	Result Inventory::fitEquipment()
	{
		int columns = baseColumns_, rows = baseRows_;
		for (const Item& worn : equipped_)
			if (worn.count > 0)
			{
				const ItemType* type = catalog_.itemType(worn.type);
				columns += type->expandColumns;
				rows += type->expandRows;
			}
		return resize(columns, rows);
	}

	Result Inventory::equipFrom(Cell cell, const std::string& slotId)
	{
		int index = placementAt(cell);
		if (index < 0)
			return Result{ Outcome::EmptyCell, 0, 0 };
		int slot = slotIndex(slotId);
		if (slot < 0)
			return Result{ Outcome::UnknownSlot, 0, 0 };
		Item item = placements_[index].item;
		item.rotated = false;
		const ItemType* type = catalog_.itemType(item.type);
		if (!contains(type->slots, slotId))
			return Result{ Outcome::NotAllowed, 0, 0 };
		if (isBlocked(slotId))
			return Result{ Outcome::Blocked, 0, 0 };

		std::vector<Placed> placementsBefore = placements_;
		std::vector<Item> equippedBefore = equipped_;
		int columnsBefore = columns_, rowsBefore = rows_;
		removeAt(index);
		std::vector<Item> displaced(1, equipped_[slot]);
		equipped_[slot] = item;
		for (const std::string& blockedId : type->blocks)
		{
			int blocked = slotIndex(blockedId);
			displaced.push_back(equipped_[blocked]);
			equipped_[blocked] = Item{};
		}
		if (fitEquipment().outcome != Outcome::Done)
		{
			restore(placementsBefore, equippedBefore, columnsBefore, rowsBefore);
			return Result{ Outcome::ItemsOutside, 0, 0 };
		}
		for (const Item& old : displaced)
			if (old.count > 0 && pickUp(old).outcome != Outcome::Done)
			{
				restore(placementsBefore, equippedBefore, columnsBefore, rowsBefore);
				return Result{ Outcome::NoRoom, 0, 0 };
			}
		return Result{ Outcome::Done, 1, 0 };
	}

	Result Inventory::unequip(const std::string& slotId)
	{
		int slot = slotIndex(slotId);
		if (slot < 0)
			return Result{ Outcome::UnknownSlot, 0, 0 };
		if (equipped_[slot].count == 0)
			return Result{ Outcome::EmptySlot, 0, 0 };
		std::vector<Placed> placementsBefore = placements_;
		std::vector<Item> equippedBefore = equipped_;
		int columnsBefore = columns_, rowsBefore = rows_;
		Item worn = equipped_[slot];
		equipped_[slot] = Item{};
		if (fitEquipment().outcome != Outcome::Done)
		{
			restore(placementsBefore, equippedBefore, columnsBefore, rowsBefore);
			return Result{ Outcome::ItemsOutside, 0, 0 };
		}
		if (pickUp(worn).outcome != Outcome::Done)
		{
			restore(placementsBefore, equippedBefore, columnsBefore, rowsBefore);
			return Result{ Outcome::NoRoom, 0, 0 };
		}
		return Result{ Outcome::Done, 1, 0 };
	}

	const Item* Inventory::itemAt(Cell cell) const
	{
		int index = placementAt(cell);
		return index < 0 ? nullptr : &placements_[index].item;
	}

	std::vector<Placed> Inventory::backpackContents() const
	{
		return placements_;
	}

	const Item* Inventory::equippedIn(const std::string& slotId) const
	{
		int slot = slotIndex(slotId);
		if (slot < 0 || equipped_[slot].count == 0)
			return nullptr;
		return &equipped_[slot];
	}

	bool Inventory::isBlocked(const std::string& slotId) const
	{
		for (const Item& item : equipped_)
			if (item.count > 0 && contains(catalog_.itemType(item.type)->blocks, slotId))
				return true;
		return false;
	}
}
