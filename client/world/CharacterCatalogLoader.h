#pragma once

#include <string>

#include "world/CharacterCatalog.h"
#include "world/ConfigError.h"

namespace world {

class CharacterCatalogLoader {
public:
	CharacterCatalog loadFromFile(const std::string& path) const;

	// Fusiona las criaturas de `path` sobre un catálogo ya cargado (p.
	// ej. characters.toml + enemies.toml): solo agrega las tablas
	// presentes en ese archivo.
	void mergeFromFile(CharacterCatalog& catalog,
			   const std::string& path) const;

	CharacterCatalog loadFromString(const std::string& toml) const;
};

}
