#include "world/ScenesRoomSource.h"

#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/SceneConfigLoader.h"
#include "world/SceneKeys.h"

namespace world {

ScenesRoomSource::ScenesRoomSource(std::string path)
	: path_(std::move(path)) {}

bool ScenesRoomSource::has(const std::string& id) const {
	try {
		const tomlcfg::TomlDocument doc(
			path_, tomlcfg::TomlDocument::Source::File);
		return doc.hasKey(std::string(kKeyRooms) + "." + id);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

SceneDescription ScenesRoomSource::load(const std::string& id) const {
	return SceneConfigLoader().loadFromFile(path_, id);
}

}
