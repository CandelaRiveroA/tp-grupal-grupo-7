#include "world/CursorConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/CursorTomlReader.h"

namespace world {

CursorCatalog CursorConfigLoader::loadFromFile(const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return CursorTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

CursorCatalog CursorConfigLoader::loadFromString(const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return CursorTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}  // namespace world
