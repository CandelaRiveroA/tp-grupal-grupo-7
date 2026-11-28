#include "world/WorldMapConfig.h"

#include <unordered_set>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/TomlConfigReader.h"

namespace world {

WorldMapConfig::WorldMapConfig(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		modal = PagedModalSpec(doc);

		const TomlConfigReader root(doc);
		const TomlConfigReader panel = root.subtable("panel");
		titleTopPad = panel.nonNegativeInt("title_top_pad");
		titleHeight = panel.positiveInt("title_height");
		titleFontSize = panel.positiveInt("title_font_size");
		mapTopGap = panel.nonNegativeInt("map_top_gap");
		mapBorder = panel.nonNegativeInt("map_border");

		const TomlConfigReader pins = root.subtable("pins");
		pinSize = pins.positiveInt("size");
		pinBorder = pins.nonNegativeInt("border");

		const TomlConfigReader text = root.subtable("text");
		unavailableText = text.nonEmptyString("unavailable");
		unavailableFontSize = text.positiveInt("unavailable_font_size");

		const TomlConfigReader colors = root.subtable("colors");
		localRingColor = colors.color("local_ring");

		const std::vector<TomlConfigReader> entries =
			root.tableArray("exteriors");
		std::unordered_set<std::string> seenIds;
		exteriors.reserve(entries.size());
		for (const TomlConfigReader& entry : entries) {
			std::string id = entry.nonEmptyString("id");
			std::string title = entry.nonEmptyString("title");
			if (!seenIds.insert(id).second) {
				throw ConfigError(ConfigErrorKind::InvalidValue,
						  entry.pathFor("id"));
			}
			exteriors.push_back(WorldMapExteriorConfig{
				std::move(id), std::move(title)});
		}
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}  // namespace world
