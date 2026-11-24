#include "world/WorldMapLoader.h"

#include "world_model/FormatError.h"
#include "world_model/WorldSerializer.h"

namespace world {

worldmodel::World WorldMapLoader::loadFromFile(
	const std::string& path) const {
	try {
		return worldmodel::WorldSerializer().loadFromFile(path);
	} catch (const worldmodel::FormatError& error) {
		throw ConfigError(error);
	}
}

worldmodel::World WorldMapLoader::loadFromString(
	const std::string& toml) const {
	try {
		return worldmodel::WorldSerializer().loadFromString(toml);
	} catch (const worldmodel::FormatError& error) {
		throw ConfigError(error);
	}
}

}
