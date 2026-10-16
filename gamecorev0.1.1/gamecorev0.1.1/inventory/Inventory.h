#pragma once
#include "Catalog.h"
#include <string>
#include <vector>

namespace inventory
{
	struct Cell
	{
		int column;
		int row;
	};

	struct Item
	{
		std::string type;
		int count;
		bool rotated = false;
	};

	struct Placed
	{
		Item item;
		Cell anchor;
	};

	enum class Outcome
	{
		Done,
		Partial,
		NoRoom,
		OutOfBounds,
		Overlaps,
		EmptyCell,
		EmptySlot,
		UnknownItem,
		UnknownSlot,
		NotAllowed,
		Blocked,
		BadCount
	};

	struct Result
	{
		Outcome outcome;
		int moved;
		int leftover;
	};

	struct Taken
	{
		Outcome outcome;
		Item item;
	};

	class Inventory
	{
	public:
		Inventory(const Catalog& catalog, int columns, int rows);
		Inventory(Catalog&& catalog, int columns, int rows) = delete;

		Result pickUp(const Item& item);
		Result placeAt(const Item& item, Cell anchor);
		Taken takeAt(Cell cell, int count);
		Result move(Cell from, Cell toAnchor);
		Result rotate(Cell cell);

		Result equipFrom(Cell cell, const std::string& slotId);
		Result unequip(const std::string& slotId);

		const Item* itemAt(Cell cell) const;
		std::vector<Placed> backpackContents() const;
		const Item* equippedIn(const std::string& slotId) const;
		bool isBlocked(const std::string& slotId) const;

	private:
		Outcome validate(const Item& item, const ItemType*& type) const;
		bool inBounds(const ItemType& type, bool rotated, Cell anchor) const;
		bool fits(const ItemType& type, bool rotated, Cell anchor) const;
		int placementAt(Cell cell) const;
		void insertInOrder(const Placed& placed);
		void removeAt(int index);
		void reindex();
		int slotIndex(const std::string& slotId) const;

		const Catalog& catalog_;
		int columns_;
		int rows_;
		std::vector<Placed> placements_;
		std::vector<int> occupant_;
		std::vector<Item> equipped_;
	};
}
