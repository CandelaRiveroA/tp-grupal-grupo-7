#pragma once

#include <string>

#include "world/ConfigError.h"
#include "world/WorldOverlay.h"

namespace world {

class OverlayLoader {
public:
	WorldOverlay loadFromFile(const std::string& path) const;

	WorldOverlay loadFromString(const std::string& toml) const;
};

}
