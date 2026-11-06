#pragma once

#include <string>

#include "world/CropCatalog.h"

namespace world {

// Carga crops.toml hacia un CropCatalog.
// Mismo patrón que ResourceConfigLoader / InventoryConfigLoader:
// TomlDocument -> reader interno (clase propia) -> datos puros.
// Lanza ConfigError si el archivo es inválido.
class CropConfigLoader {
public:
	CropCatalog loadFromFile(const std::string& path) const;

	CropCatalog loadFromString(const std::string& toml) const;
};

}
