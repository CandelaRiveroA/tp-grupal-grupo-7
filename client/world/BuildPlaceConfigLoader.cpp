#include "world/BuildPlaceConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/BuildPlaceTomlReader.h"
#include "world/ConfigError.h"

namespace world {

BuildPlaceCatalog BuildPlaceConfigLoader::loadFromFile(
	const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return BuildPlaceTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

BuildPlaceCatalog BuildPlaceConfigLoader::loadFromString(
	const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return BuildPlaceTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
