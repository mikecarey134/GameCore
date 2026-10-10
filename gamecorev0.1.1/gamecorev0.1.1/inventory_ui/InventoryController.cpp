#include "InventoryController.h"

namespace inventory
{
	InventoryController::InventoryController(Inventory& inventory, const Catalog& catalog)
		: inventory_(inventory), catalog_(catalog)
	{
	}

	int InventoryController::cellsUsed() const
	{
		int used = 0;
		for (const Placed& placed : inventory_.backpackContents())
		{
			const ItemType* type = catalog_.itemType(placed.item.type);
			used += type->width * type->height;
		}
		return used;
	}

	void InventoryController::leftClick(const Hit& hit)
	{
		message_.clear();
		if (hit.kind == Hit::Kind::None)
			return;
		if (holding_)
			drop(hit);
		else if (hit.kind == Hit::Kind::Slot)
			unequipFrom(hit);
		else
			pickUpAt(hit.cell);
	}

	void InventoryController::rightClick()
	{
		holding_ = false;
		message_.clear();
	}

	void InventoryController::drop(const Hit& hit)
	{
		Result result = hit.kind == Hit::Kind::Slot
			? inventory_.equipFrom(heldFrom_, catalog_.slotId(hit.slot))
			: inventory_.move(heldFrom_, Cell{ hit.cell.column - grabOffset_.column, hit.cell.row - grabOffset_.row });
		if (result.outcome == Outcome::Done || result.outcome == Outcome::Partial)
			holding_ = false;
		else if (result.outcome == Outcome::EmptyCell)
		{
			holding_ = false;
			message_ = "That item is no longer there";
		}
		else
			explain(result.outcome, hit);
	}

	void InventoryController::unequipFrom(const Hit& hit)
	{
		Result result = inventory_.unequip(catalog_.slotId(hit.slot));
		if (result.outcome == Outcome::NoRoom)
			explain(result.outcome, hit);
	}

	void InventoryController::pickUpAt(Cell cell)
	{
		for (const Placed& placed : inventory_.backpackContents())
		{
			const ItemType* type = catalog_.itemType(placed.item.type);
			Cell offset{ cell.column - placed.anchor.column, cell.row - placed.anchor.row };
			if (offset.column >= 0 && offset.row >= 0 && offset.column < type->width && offset.row < type->height)
			{
				holding_ = true;
				heldFrom_ = placed.anchor;
				grabOffset_ = offset;
				return;
			}
		}
	}

	void InventoryController::explain(Outcome outcome, const Hit& hit)
	{
		if (outcome == Outcome::NotAllowed)
		{
			const std::string& name = catalog_.itemType(inventory_.itemAt(heldFrom_)->type)->name;
			bool vowel = !name.empty() && std::string("AEIOUaeiou").find(name[0]) != std::string::npos;
			message_ = (vowel ? "An " : "A ") + name + " can't go in " + catalog_.slotName(hit.slot);
		}
		else if (outcome == Outcome::NoRoom && hit.kind == Hit::Kind::Cell)
			message_ = "That stack is full";
		else if (outcome == Outcome::NoRoom)
			message_ = "No room in the backpack";
		else if (outcome == Outcome::Blocked)
			message_ = catalog_.slotName(hit.slot) + " is blocked";
		else if (outcome == Outcome::OutOfBounds)
			message_ = "It doesn't fit there";
		else if (outcome == Outcome::Overlaps)
			message_ = "Something is in the way";
		else
			message_ = "That can't be done";
	}
}
