#include "world/CropConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/CropTomlReader.h"

namespace world {

CropCatalog CropConfigLoader::loadFromFile(const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return CropTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

CropCatalog CropConfigLoader::loadFromString(const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return CropTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
