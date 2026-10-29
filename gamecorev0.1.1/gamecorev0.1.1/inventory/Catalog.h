#pragma once
#include <string>
#include <vector>

namespace inventory
{
	enum class Category { Weapon, Armor, Accessory, Consumable, KeyItem, Tool };

	struct ItemType
	{
		std::string id;
		std::string name;
		int width = 1;
		int height = 1;
		std::vector<std::string> slots;
		std::vector<std::string> blocks;
		int maxStack = 1;
		Category category = Category::Tool;
		std::string icon;
		int expandColumns = 0;
		int expandRows = 0;
	};

	struct LoadError
	{
		std::string file;
		int line = 0;
		std::string message;
	};

	struct Slot
	{
		std::string id;
		std::string name;
		std::string icon;
		int dollColumn = -1;
		int dollRow = -1;
	};

	class Catalog
	{
	public:
		int slotCount() const { return (int)slots_.size(); }
		const std::string& slotId(int index) const { return slots_[index].id; }
		const std::string& slotName(int index) const { return slots_[index].name; }
		const Slot& slot(int index) const { return slots_[index]; }
		bool hasSlot(const std::string& id) const;
		const ItemType* itemType(const std::string& id) const;

	private:
		friend bool loadCatalog(const std::string&, const std::string&, Catalog&, LoadError&);

		std::vector<Slot> slots_;
		std::vector<ItemType> itemTypes_;
	};

	bool loadCatalog(const std::string& slotsCsv, const std::string& itemsCsv,
		Catalog& catalog, LoadError& error);
}
