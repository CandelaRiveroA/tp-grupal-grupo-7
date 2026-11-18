#include "world_model/PrefabCatalog.h"

#include <unordered_set>
#include <utility>

namespace worldmodel {

void PrefabCatalog::setTilesets(std::map<std::string, TilesetDef> tilesets) {
	tilesets_ = std::move(tilesets);
}

void PrefabCatalog::reserve(std::size_t count) {
	prefabs_.reserve(count);
}

void PrefabCatalog::add(PrefabDef prefab) {
	prefabs_.push_back(std::move(prefab));
}

const std::map<std::string, TilesetDef>& PrefabCatalog::tilesets() const {
	return tilesets_;
}

std::vector<std::pair<std::string, std::string>>
PrefabCatalog::categories() const {
	std::vector<std::pair<std::string, std::string>> result;
	std::unordered_set<std::string> seen;
	for (const PrefabDef& prefab : prefabs_) {
		if (seen.insert(prefab.category).second) {
			result.emplace_back(prefab.category,
					    prefab.categoryLabel);
		}
	}
	return result;
}

std::vector<const PrefabDef*> PrefabCatalog::byCategory(
	const std::string& category) const {
	std::vector<const PrefabDef*> result;
	for (const PrefabDef& prefab : prefabs_) {
		if (prefab.category == category) {
			result.push_back(&prefab);
		}
	}
	return result;
}

const PrefabDef* PrefabCatalog::find(const std::string& id) const {
	for (const PrefabDef& prefab : prefabs_) {
		if (prefab.id == id) {
			return &prefab;
		}
	}
	return nullptr;
}

}
