#include "Catalog.h"
#include <sstream>

namespace inventory
{
	namespace
	{
		std::string trim(const std::string& text)
		{
			const char* space = " \t\r\n";
			std::string::size_type first = text.find_first_not_of(space);
			if (first == std::string::npos)
				return "";
			return text.substr(first, text.find_last_not_of(space) - first + 1);
		}

		std::vector<std::string> split(const std::string& text, char separator)
		{
			std::vector<std::string> parts;
			std::string::size_type start = 0;
			while (true)
			{
				std::string::size_type end = text.find(separator, start);
				parts.push_back(trim(text.substr(start, end == std::string::npos ? std::string::npos : end - start)));
				if (end == std::string::npos)
					return parts;
				start = end + 1;
			}
		}

		struct Row
		{
			std::vector<std::string> cells;
			int line;
		};

		struct Table
		{
			std::vector<std::string> header;
			int headerLine = 0;
			std::vector<Row> rows;

			int column(const std::string& name) const
			{
				for (std::string::size_type i = 0; i < header.size(); ++i)
					if (header[i] == name)
						return (int)i;
				return -1;
			}
		};

		Table readTable(const std::string& csv)
		{
			Table table;
			std::istringstream in(csv);
			std::string line;
			int lineNumber = 0;
			while (std::getline(in, line))
			{
				++lineNumber;
				if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
					line.erase(0, 3);
				if (trim(line).empty())
					continue;
				if (table.header.empty())
				{
					table.header = split(line, ',');
					table.headerLine = lineNumber;
				}
				else
					table.rows.push_back(Row{ split(line, ','), lineNumber });
			}
			return table;
		}

		class FileErrors
		{
		public:
			FileErrors(const char* file, LoadError& error) : file_(file), error_(error) {}

			bool at(int line, const std::string& message)
			{
				error_.file = file_;
				error_.line = line;
				error_.message = message;
				return false;
			}

		private:
			const char* file_;
			LoadError& error_;
		};

		bool checkShape(const Table& table, const std::vector<std::string>& required, FileErrors& errors)
		{
			for (std::string::size_type i = 0; i < table.header.size(); ++i)
				for (std::string::size_type j = i + 1; j < table.header.size(); ++j)
					if (table.header[i] == table.header[j])
						return errors.at(table.headerLine, "column '" + table.header[i] + "' appears twice");
			for (const std::string& name : required)
				if (table.column(name) < 0)
					return errors.at(table.headerLine, "missing column '" + name + "'");
			for (const Row& row : table.rows)
				if (row.cells.size() != table.header.size())
					return errors.at(row.line, std::to_string(row.cells.size()) + " cells, but the header has "
						+ std::to_string(table.header.size()));
			return true;
		}

		bool parseCount(const std::string& text, int& value)
		{
			if (text.empty() || text.size() > 9 || text.find_first_not_of("0123456789") != std::string::npos)
				return false;
			value = std::stoi(text);
			return value >= 1;
		}

		bool parseIndex(const std::string& text, int& value)
		{
			if (text.empty() || text.size() > 9 || text.find_first_not_of("0123456789") != std::string::npos)
				return false;
			value = std::stoi(text);
			return true;
		}

		bool parseCategory(const std::string& text, Category& value)
		{
			if (text == "weapon") value = Category::Weapon;
			else if (text == "armor") value = Category::Armor;
			else if (text == "accessory") value = Category::Accessory;
			else if (text == "consumable") value = Category::Consumable;
			else if (text == "key_item") value = Category::KeyItem;
			else if (text == "tool") value = Category::Tool;
			else return false;
			return true;
		}

		bool loadSlots(const std::string& csv, std::vector<Slot>& slots, LoadError& error)
		{
			FileErrors errors("slots.csv", error);
			Table table = readTable(csv);
			if (!checkShape(table, { "id", "name" }, errors))
				return false;
			if (table.rows.empty())
				return errors.at(0, "no slots");
			bool hasDoll = table.column("doll_column") >= 0 || table.column("doll_row") >= 0;
			if (hasDoll && !checkShape(table, { "doll_column", "doll_row" }, errors))
				return false;

			int id = table.column("id");
			int name = table.column("name");
			for (const Row& row : table.rows)
			{
				const std::string& slotId = row.cells[id];
				if (slotId.empty())
					return errors.at(row.line, "empty slot id");
				for (const Slot& slot : slots)
					if (slot.id == slotId)
						return errors.at(row.line, "duplicate slot id '" + slotId + "'");
				Slot slot;
				slot.id = slotId;
				slot.name = row.cells[name];
				if (table.column("icon") >= 0 && row.cells[table.column("icon")] != "-")
					slot.icon = row.cells[table.column("icon")];
				if (hasDoll)
				{
					if (!parseIndex(row.cells[table.column("doll_column")], slot.dollColumn))
						return errors.at(row.line, "doll_column must be a whole number of at least 0");
					if (!parseIndex(row.cells[table.column("doll_row")], slot.dollRow))
						return errors.at(row.line, "doll_row must be a whole number of at least 0");
				}
				slots.push_back(slot);
			}
			return true;
		}

		bool parseSlotList(const std::string& column, const std::string& cell, const Catalog& catalog,
			int line, std::vector<std::string>& slots, FileErrors& errors)
		{
			if (cell.empty())
				return errors.at(line, column + " is empty; write '-' for none");
			if (cell == "-")
				return true;
			slots = split(cell, '|');
			for (const std::string& slot : slots)
				if (!catalog.hasSlot(slot))
					return errors.at(line, "unknown slot '" + slot + "'");
			return true;
		}
	}

	bool Catalog::hasSlot(const std::string& id) const
	{
		for (const Slot& slot : slots_)
			if (slot.id == id)
				return true;
		return false;
	}

	const ItemType* Catalog::itemType(const std::string& id) const
	{
		for (const ItemType& type : itemTypes_)
			if (type.id == id)
				return &type;
		return nullptr;
	}

	bool loadCatalog(const std::string& slotsCsv, const std::string& itemsCsv,
		Catalog& catalog, LoadError& error)
	{
		catalog = Catalog();
		Catalog loaded;
		if (!loadSlots(slotsCsv, loaded.slots_, error))
			return false;

		FileErrors errors("items.csv", error);
		Table table = readTable(itemsCsv);
		if (!checkShape(table, { "id", "name", "width", "height", "slots", "blocks", "max_stack", "category" }, errors))
			return false;
		if (table.rows.empty())
			return errors.at(0, "no item types");

		for (const Row& row : table.rows)
		{
			auto cell = [&](const char* name) -> const std::string& { return row.cells[table.column(name)]; };

			ItemType type;
			type.id = cell("id");
			type.name = cell("name");
			if (type.id.empty())
				return errors.at(row.line, "empty item id");
			if (loaded.itemType(type.id))
				return errors.at(row.line, "duplicate item id '" + type.id + "'");

			if (!parseCount(cell("width"), type.width))
				return errors.at(row.line, "width must be a whole number of at least 1");
			if (!parseCount(cell("height"), type.height))
				return errors.at(row.line, "height must be a whole number of at least 1");
			if (!parseCount(cell("max_stack"), type.maxStack))
				return errors.at(row.line, "max_stack must be a whole number of at least 1");
			if (!parseCategory(cell("category"), type.category))
				return errors.at(row.line, "unknown category '" + cell("category") + "'");

			if (!parseSlotList("slots", cell("slots"), loaded, row.line, type.slots, errors)
				|| !parseSlotList("blocks", cell("blocks"), loaded, row.line, type.blocks, errors))
				return false;

			if (type.maxStack > 1 && type.category != Category::Consumable)
				return errors.at(row.line, "'" + type.id + "' stacks but is not a consumable");
			if (type.maxStack > 1 && !type.slots.empty())
				return errors.at(row.line, "'" + type.id + "' stacks, so it cannot be equipped");
			if (!type.blocks.empty() && type.slots.empty())
				return errors.at(row.line, "'" + type.id + "' blocks slots but cannot be equipped");
			for (const std::string& blocked : type.blocks)
				for (const std::string& slot : type.slots)
					if (blocked == slot)
						return errors.at(row.line, "'" + type.id + "' blocks " + slot + ", a slot it can be equipped in");

			if (table.column("icon") >= 0)
			{
				if (cell("icon").empty())
					return errors.at(row.line, "icon is empty; write '-' for none");
				if (cell("icon") != "-")
					type.icon = cell("icon");
			}

			loaded.itemTypes_.push_back(type);
		}

		catalog = loaded;
		return true;
	}
}
