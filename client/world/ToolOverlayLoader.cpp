#include "world/ToolOverlayLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/ToolOverlayTomlReader.h"

namespace world {

ToolOverlayCatalog ToolOverlayLoader::loadFromFile(
	const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return ToolOverlayTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

ToolOverlayCatalog ToolOverlayLoader::loadFromString(
	const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return ToolOverlayTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
