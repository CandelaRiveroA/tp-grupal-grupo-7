#pragma once

#include <string>

#include "world/ConfigError.h"
#include "world/SceneDescription.h"

namespace world {

class SceneConfigLoader {
public:
	SceneDescription loadFromFile(const std::string& path,
				      const std::string& roomName) const;

	SceneDescription loadFromString(const std::string& toml,
					const std::string& roomName) const;
};

}
