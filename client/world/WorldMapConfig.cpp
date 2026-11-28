#include "world/WorldMapConfig.h"

#include <array>
#include <unordered_set>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

WorldMapConfig::WorldMapConfig(const std::string& path) {
    try {
        const tomlcfg::TomlDocument doc(path, tomlcfg::TomlDocument::Source::File);
        loadPanel(requireTable(doc, "panel"));
        loadArrows(requireTable(doc, "arrows"));
        loadPins(requireTable(doc, "pins"));
        loadText(requireTable(doc, "text"));
        loadColors(requireTable(doc, "colors"));
        loadExteriors(doc);
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
}

void WorldMapConfig::loadPanel(const tomlcfg::TomlDocument& panel) {
    widthFraction = readFraction(panel, "width_fraction");
    heightFraction = readFraction(panel, "height_fraction");

    const std::array<int, 2> minSize = panel.getIntArray<2>("min_size");
    minWidth = minSize[0];
    minHeight = minSize[1];
    if (minWidth <= 0 || minHeight <= 0) {
        throw ConfigError(ConfigErrorKind::InvalidValue, "panel.min_size");
    }

    framePad = readNonNegative(panel, "frame_pad");
    titleTopPad = readNonNegative(panel, "title_top_pad");
    titleHeight = readPositive(panel, "title_height");
    titleFontSize = readPositive(panel, "title_font_size");
    bgInset = readNonNegative(panel, "bg_inset");
    mapTopGap = readNonNegative(panel, "map_top_gap");
    mapBorder = readNonNegative(panel, "map_border");
}

void WorldMapConfig::loadArrows(const tomlcfg::TomlDocument& arrows) {
    const std::array<int, 2> size = arrows.getIntArray<2>("size");
    arrowWidth = size[0];
    arrowHeight = size[1];
    if (arrowWidth <= 0 || arrowHeight <= 0) {
        throw ConfigError(ConfigErrorKind::InvalidValue, "arrows.size");
    }

    arrowMargin = readNonNegative(arrows, "margin");
    arrowGap = readNonNegative(arrows, "gap");
    arrowLeftSheet = readNonEmpty(arrows, "left_sheet");
    arrowRightSheet = readNonEmpty(arrows, "right_sheet");
    arrowLeftFallback = readNonEmpty(arrows, "left_fallback");
    arrowRightFallback = readNonEmpty(arrows, "right_fallback");
    arrowFontSize = readPositive(arrows, "font_size");
}

void WorldMapConfig::loadPins(const tomlcfg::TomlDocument& pins) {
    pinSize = readPositive(pins, "size");
    pinBorder = readNonNegative(pins, "border");
    if (pins.hasKey("label_font")) {
        pinLabel.font = readNonEmpty(pins, "label_font");
    }
    pinLabel.fontSize = readPositive(pins, "label_font_size");
    if (pins.hasKey("label_bold")) {
        pinLabel.bold = pins.getBool("label_bold");
    }
    pinLabel.gap = readNonNegative(pins, "label_gap");
    pinLabel.height = readPositive(pins, "label_height");
    pinLabel.padX = readNonNegative(pins, "label_pad_x");
}

void WorldMapConfig::loadText(const tomlcfg::TomlDocument& text) {
    unavailableText = readNonEmpty(text, "unavailable");
    unavailableFontSize = readPositive(text, "unavailable_font_size");
}

void WorldMapConfig::loadColors(const tomlcfg::TomlDocument& colors) {
    backdropColor = readColor(colors, "backdrop");
    parchmentColor = readColor(colors, "parchment");
    woodBorderColor = readColor(colors, "wood_border");
    localRingColor = readColor(colors, "local_ring");
    inkColor = readColor(colors, "ink");
}

void WorldMapConfig::loadExteriors(const tomlcfg::TomlDocument& doc) {
    const std::vector<tomlcfg::TomlDocument> entries = doc.getTableArray("exteriors");
    if (entries.empty()) {
        throw ConfigError(ConfigErrorKind::MissingStructure, "exteriors");
    }

    std::unordered_set<std::string> seenIds;
    exteriors.reserve(entries.size());
    for (const tomlcfg::TomlDocument& entry : entries) {
        std::string id = readNonEmpty(entry, "id");
        std::string title = readNonEmpty(entry, "title");
        if (!seenIds.insert(id).second) {
            throw ConfigError(ConfigErrorKind::InvalidValue, id);
        }
        exteriors.push_back(WorldMapExteriorConfig{std::move(id), std::move(title)});
    }
}

Color WorldMapConfig::readColor(const tomlcfg::TomlDocument& colors,
                                const std::string& key) const {
    const std::array<int, 4> rgba = colors.getIntArray<4>(key);
    for (const int channel : rgba) {
        if (channel < 0 || channel > 255) {
            throw ConfigError(ConfigErrorKind::InvalidValue, key);
        }
    }
    return Color{rgba[0], rgba[1], rgba[2], rgba[3]};
}

int WorldMapConfig::readPositive(const tomlcfg::TomlDocument& doc,
                                 const std::string& key) const {
    const int value = doc.getInt(key);
    if (value <= 0) {
        throw ConfigError(ConfigErrorKind::InvalidValue, key);
    }
    return value;
}

int WorldMapConfig::readNonNegative(const tomlcfg::TomlDocument& doc,
                                    const std::string& key) const {
    const int value = doc.getInt(key);
    if (value < 0) {
        throw ConfigError(ConfigErrorKind::InvalidValue, key);
    }
    return value;
}

double WorldMapConfig::readFraction(const tomlcfg::TomlDocument& doc,
                                    const std::string& key) const {
    const double value = doc.getDouble(key);
    if (value <= 0.0 || value > 1.0) {
        throw ConfigError(ConfigErrorKind::InvalidValue, key);
    }
    return value;
}

std::string WorldMapConfig::readNonEmpty(const tomlcfg::TomlDocument& doc,
                                         const std::string& key) const {
    std::string value = doc.getString(key);
    if (value.empty()) {
        throw ConfigError(ConfigErrorKind::InvalidValue, key);
    }
    return value;
}

tomlcfg::TomlDocument WorldMapConfig::requireTable(const tomlcfg::TomlDocument& doc,
                                                   const std::string& key) const {
    if (!doc.hasKey(key)) {
        throw ConfigError(ConfigErrorKind::MissingStructure, key);
    }
    return doc.getSubtable(key);
}

}  // namespace world
