#pragma once

#include <string>

#include "world/ConfigError.h"
#include "world_model/World.h"

namespace world {

// Lee el TOML del mundo que guarda el editor (common/world_model) y
// traduce sus errores a ConfigError. El cliente solo LEE: no valida las
// reglas de edición (WorldValidator es del editor); el reader del
// modelo ya chequea tamaños de grilla, tilesets y vocabulario.
class WorldMapLoader {
public:
	worldmodel::World loadFromFile(const std::string& path) const;

	worldmodel::World loadFromString(const std::string& toml) const;
};

}
