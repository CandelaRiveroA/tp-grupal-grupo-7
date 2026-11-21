#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "world_model/Tile.h"

namespace worldmodel {

struct PrefabDef {
	std::string id;
	std::string name;
	std::string category;
	std::string categoryLabel;
	TileRef tile;
	int width = 1;
	int height = 1;
	std::vector<bool> collisionGrid;
};

class PrefabCatalog {
public:
	void setTilesets(std::map<std::string, TilesetDef> tilesets);
	void reserve(std::size_t count);
	void add(PrefabDef prefab);

	const std::map<std::string, TilesetDef>& tilesets() const;
	std::vector<std::pair<std::string, std::string>> categories() const;
	std::vector<const PrefabDef*> byCategory(
		const std::string& category) const;
	const PrefabDef* find(const std::string& id) const;

private:
	std::map<std::string, TilesetDef> tilesets_;
	std::vector<PrefabDef> prefabs_;
};

}
