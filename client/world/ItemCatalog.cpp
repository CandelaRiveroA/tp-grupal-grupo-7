#include "world/ItemCatalog.h"

namespace world {

bool ItemCatalog::has(ItemKind kind) const {
	return visuals.find(kind) != visuals.end();
}

const ItemVisual& ItemCatalog::visualFor(ItemKind kind) const {
	return visuals.at(kind);
}

}
