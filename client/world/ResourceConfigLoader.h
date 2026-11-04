#pragma once

#include <string>

#include "world/ResourceCatalog.h"

namespace world {

// Carga resources.toml hacia un ResourceCatalog.
// Mismo patrón que InventoryConfigLoader / ToolOverlayLoader:
// TomlDocument -> reader interno (clase propia) -> datos puros.
// Lanza ConfigError si el archivo es inválido.
class ResourceConfigLoader {
public:
	ResourceCatalog loadFromFile(const std::string& path) const;

	ResourceCatalog loadFromString(const std::string& toml) const;
};

}
