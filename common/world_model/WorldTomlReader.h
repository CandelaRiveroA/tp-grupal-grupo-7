#pragma once

#include <cstddef>
#include <string>

#include "toml/TomlDocument.h"
#include "world_model/Map.h"
#include "world_model/World.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class WorldTomlReader {
public:
	explicit WorldTomlReader(const tomlcfg::TomlDocument& doc);

	World read() const;

private:
	std::string join(const std::string& base, const std::string& key) const;
	std::string idx(const std::string& base, std::size_t index) const;
	void readWorld(World& world) const;
	void readTilesets(World& world) const;
	Map readMap(const World& world, const std::string& id) const;
	void readObjects(const World& world, const std::string& mapPath,
			 Map& map) const;
	void readMarkers(const std::string& mapPath, Map& map) const;
	void readPortals(const std::string& mapPath, Map& map) const;
	TileGrid readGrid(const World& world, const std::string& path,
			  int width, int height) const;
	void checkTileset(const World& world, const TileRef& tile,
			  const std::string& path) const;

	const tomlcfg::TomlDocument& doc_;
	WorldVocabulary vocabulary_;
};

}
