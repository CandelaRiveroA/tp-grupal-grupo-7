#include "world/SceneConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/SceneTomlReader.h"

namespace world {


SceneDescription SceneConfigLoader::loadFromString(
	const std::string& toml, const std::string& roomName) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return SceneTomlReader(doc).readRoom(roomName);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

SceneDescription SceneConfigLoader::loadFromFile(
	const std::string& path, const std::string& roomName) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return SceneTomlReader(doc).readRoom(roomName);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
