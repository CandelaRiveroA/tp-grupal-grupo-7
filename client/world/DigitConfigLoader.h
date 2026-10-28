#pragma once

#include <string>

#include "world/DigitCatalog.h"

namespace world {

// Carga digits.toml hacia un DigitCatalog. Mismo patrón que
// ResourceConfigLoader / InventoryConfigLoader: TomlDocument -> reader
// interno -> datos puros. Lanza ConfigError si el archivo es inválido.
class DigitConfigLoader {
public:
	DigitCatalog loadFromFile(const std::string& path) const;

	DigitCatalog loadFromString(const std::string& toml) const;
};

}  // namespace world
