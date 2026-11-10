#pragma once

#include <string>

#include "toml/TomlDocument.h"
#include "world_model/Map.h"
#include "world_model/FieldReader.h"
#include "world_model/World.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class WorldTomlReader {
public:
	explicit WorldTomlReader(const tomlcfg::TomlDocument& doc);

	World read() const;

private:
	void readWorld(World& world) const;
	Map readMap(const World& world, const std::string& id) const;
	void readObjects(const World& world, const std::string& mapPath,
			 Map& map) const;
	void readMarkers(const std::string& mapPath, Map& map) const;
	void readPortals(const std::string& mapPath, Map& map) const;

	const tomlcfg::TomlDocument& doc_;
	FieldReader fields_;
	WorldVocabulary vocabulary_;
};

}
