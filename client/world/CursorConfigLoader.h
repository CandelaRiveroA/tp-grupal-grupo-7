#pragma once

#include <string>

#include "world/CursorCatalog.h"

namespace world {

// Carga ui/cursors.toml hacia un CursorCatalog. Mismo patrón que
// DigitConfigLoader / ResourceConfigLoader: TomlDocument -> reader
// interno -> datos puros. Lanza ConfigError si el archivo es inválido.
class CursorConfigLoader {
public:
	CursorCatalog loadFromFile(const std::string& path) const;

	CursorCatalog loadFromString(const std::string& toml) const;
};

}  // namespace world
