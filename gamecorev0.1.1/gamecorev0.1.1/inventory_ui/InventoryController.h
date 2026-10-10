#pragma once
#include "Catalog.h"
#include "Inventory.h"
#include "InventoryLayout.h"
#include <string>

namespace inventory
{
	class InventoryController
	{
	public:
		InventoryController(Inventory& inventory, const Catalog& catalog);

		void leftClick(const Hit& hit);
		void rightClick();

		bool isHolding() const { return holding_; }
		Cell heldFrom() const { return heldFrom_; }
		Cell grabOffset() const { return grabOffset_; }
		const std::string& message() const { return message_; }
		int cellsUsed() const;

	private:
		void drop(const Hit& hit);
		void unequipFrom(const Hit& hit);
		void pickUpAt(Cell cell);
		void explain(Outcome outcome, const Hit& hit);

		Inventory& inventory_;
		const Catalog& catalog_;
		std::string message_;
		bool holding_ = false;
		Cell heldFrom_{ 0, 0 };
		Cell grabOffset_{ 0, 0 };
	};
}
