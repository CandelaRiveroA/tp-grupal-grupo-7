#pragma once

#include <string>

#include "world_model/World.h"

namespace worldmodel {

class WorldSerializer {
public:
	World loadFromFile(const std::string& path) const;
	World loadFromString(const std::string& toml) const;
	void saveToFile(const World& world, const std::string& path) const;
	std::string saveToString(const World& world) const;
};

}
