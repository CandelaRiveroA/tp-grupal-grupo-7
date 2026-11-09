#pragma once

#include <string>

#include "world/DialogCatalog.h"

namespace world {

// Carga ui/dialogs.toml hacia un DialogCatalog. Mismo patrón que
// CursorConfigLoader / ResourceConfigLoader: TomlDocument -> reader
// interno -> datos puros. Lanza ConfigError si el archivo es inválido.
class DialogConfigLoader {
public:
	DialogCatalog loadFromFile(const std::string& path) const;

	DialogCatalog loadFromString(const std::string& toml) const;
};

}  // namespace world
