#include "world/DigitConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/DigitTomlReader.h"

namespace world {

DigitCatalog DigitConfigLoader::loadFromFile(const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return DigitTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

DigitCatalog DigitConfigLoader::loadFromString(const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return DigitTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}  // namespace world
