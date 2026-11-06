#pragma once

#include <string>

#include "world/GroundCatalog.h"

namespace world {

// Carga ground.toml hacia un GroundCatalog.
// Mismo patrón que CropConfigLoader / ResourceConfigLoader:
// TomlDocument -> reader interno (clase propia) -> datos puros.
// Lanza ConfigError si el archivo es inválido.
class GroundConfigLoader {
public:
	GroundCatalog loadFromFile(const std::string& path) const;

	GroundCatalog loadFromString(const std::string& toml) const;
};

}
