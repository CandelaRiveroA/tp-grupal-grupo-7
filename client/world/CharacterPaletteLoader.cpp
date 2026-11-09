#include "world/CharacterPaletteLoader.h"

#include <cstddef>
#include <string>
#include <vector>

#include "world/ConfigError.h"

namespace world {

namespace {
constexpr int kMaxChannel = 255;
constexpr const char* kEyesTable = "eyes";
constexpr const char* kHairTable = "hair";
constexpr const char* kPantsTable = "pants";
constexpr const char* kColorKey = "color";
}

CharacterPalettes CharacterPaletteLoader::loadFromString(
	const std::string& toml) const {
	try {
		const tomlcfg::TomlDocument doc(
			toml, tomlcfg::TomlDocument::Source::Text);
		return read(doc);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

CharacterPalettes CharacterPaletteLoader::loadFromFile(
	const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return read(doc);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

CharacterPalettes CharacterPaletteLoader::read(
	const tomlcfg::TomlDocument& doc) const {
	CharacterPalettes palettes;
	palettes.eyes = readColors(doc, kEyesTable);
	palettes.hair = readColors(doc, kHairTable);
	palettes.pants = readColors(doc, kPantsTable);
	return palettes;
}

std::vector<std::array<int, 3>> CharacterPaletteLoader::readColors(
	const tomlcfg::TomlDocument& doc, const std::string& table) const {
	std::vector<std::array<int, 3>> colors;
	const std::vector<tomlcfg::TomlDocument> entries =
		doc.getTableArray(table);
	for (std::size_t i = 0; i < entries.size(); ++i) {
		const std::string path = table + "[" + std::to_string(i) + "]." +
					 kColorKey;
		const std::array<int, 3> color =
			entries[i].getIntArray<3>(kColorKey);
		for (const int channel : color) {
			if (channel < 0 || channel > kMaxChannel) {
				throw ConfigError(ConfigErrorKind::InvalidValue, path);
			}
		}
		colors.push_back(color);
	}
	return colors;
}

}
