#pragma once

#include <string>

#include "world/ConfigError.h"
#include "world/InventoryLayout.h"

namespace world {

class InventoryConfigLoader {
public:
	InventoryLayout loadFromFile(const std::string& path) const;

	InventoryLayout loadFromString(const std::string& toml) const;
};

}
