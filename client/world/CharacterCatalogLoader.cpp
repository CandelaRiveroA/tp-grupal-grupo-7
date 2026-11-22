#include "world/CharacterCatalogLoader.h"

#include "toml/TomlDocument.h"
#include "world/CharacterTomlReader.h"
#include "world/ConfigError.h"

namespace world {

CharacterCatalog CharacterCatalogLoader::loadFromString(
	const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return CharacterTomlReader(doc).readCatalog();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

CharacterCatalog CharacterCatalogLoader::loadFromFile(
	const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return CharacterTomlReader(doc).readCatalog();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

void CharacterCatalogLoader::mergeFromFile(CharacterCatalog& catalog,
					   const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		CharacterTomlReader(doc).mergeInto(catalog);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
