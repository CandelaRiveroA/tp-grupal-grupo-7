#pragma once

#include <map>
#include <string>

namespace world {

enum class ItemKind {
	Shovel,
	Pickaxe,
	WateringCan,
	Sword,
	Egg,
	Milk,
};

struct ItemVisual {
	std::string sheetImage;
	int col = 0;
	int row = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	int drawnWidth = 0;
	int drawnHeight = 0;
};

struct ItemCatalog {
	std::map<ItemKind, ItemVisual> visuals;

	bool has(ItemKind kind) const;

	const ItemVisual& visualFor(ItemKind kind) const;
};

}
