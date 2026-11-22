#pragma once

#include <string>

#include <toml++/toml.hpp>

#include "world_model/World.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class WorldTomlWriter {
public:
	std::string write(const World& world) const;

private:
	toml::table writeWorld(const World& world) const;
	toml::table writeMap(const Map& map) const;
	toml::array writeObjects(const Map& map) const;
	toml::array writeMarkers(const Map& map) const;
	toml::array writePortals(const Map& map) const;

	WorldVocabulary vocabulary_;
};

}
