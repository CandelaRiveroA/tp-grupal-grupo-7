#pragma once

#include <string>

#include "world/CharacterCatalog.h"
#include "world/ConfigError.h"

namespace world {

class CharacterCatalogLoader {
public:
	CharacterCatalog loadFromFile(const std::string& path) const;

	CharacterCatalog loadFromString(const std::string& toml) const;
};

}
