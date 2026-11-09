#include "world/GroundConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/GroundTomlReader.h"

namespace world {

GroundCatalog GroundConfigLoader::loadFromFile(const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return GroundTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

GroundCatalog GroundConfigLoader::loadFromString(const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return GroundTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
