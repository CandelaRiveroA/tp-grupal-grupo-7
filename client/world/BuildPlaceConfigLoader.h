#pragma once

#include <string>

#include "world/BuildPlaceCatalog.h"

namespace world {

// Carga buildplace.toml hacia un BuildPlaceCatalog.
// Mismo patrón que GroundConfigLoader / CropConfigLoader.
// Lanza ConfigError si el archivo es inválido.
class BuildPlaceConfigLoader {
public:
	BuildPlaceCatalog loadFromFile(const std::string& path) const;

	BuildPlaceCatalog loadFromString(const std::string& toml) const;
};

}
