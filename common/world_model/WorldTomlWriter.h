#pragma once

#include <string>
#include <vector>

#include <toml++/toml.hpp>

#include "world_model/World.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class WorldTomlWriter {
public:
	std::string write(const World& world) const;

private:
	toml::table writeWorld(const World& world) const;
	toml::table writeTilesets(const World& world) const;
	toml::table writeMap(const Map& map) const;
	toml::array writeObjects(const Map& map) const;
	toml::array writeMarkers(const Map& map) const;
	toml::array writePortals(const Map& map) const;
	toml::array writeGrid(const TileGrid& grid) const;
	toml::array writeBooleans(const std::vector<bool>& values) const;
	toml::array writePair(int first, int second) const;

	WorldVocabulary vocabulary_;
};

}
