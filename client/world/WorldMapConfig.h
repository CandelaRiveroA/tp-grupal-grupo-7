#pragma once

#include <string>
#include <vector>

#include "world/Color.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

struct WorldMapExteriorConfig {
    std::string id;
    std::string title;
};

// Configuración de presentación del mapa del mundo cargada desde el TOML
struct WorldMapConfig {
    explicit WorldMapConfig(const std::string& path);

    double widthFraction = 0.0;
    double heightFraction = 0.0;
    int minWidth = 0;
    int minHeight = 0;
    int framePad = 0;
    int titleTopPad = 0;
    int titleHeight = 0;
    int titleFontSize = 0;
    int bgInset = 0;
    int mapTopGap = 0;
    int mapBorder = 0;
    int arrowWidth = 0;
    int arrowHeight = 0;
    int arrowMargin = 0;
    int arrowGap = 0;
    std::string arrowLeftSheet;
    std::string arrowRightSheet;
    std::string arrowLeftFallback;
    std::string arrowRightFallback;
    int arrowFontSize = 0;
    std::string unavailableText;
    int unavailableFontSize = 0;
    int pinSize = 0;
    int pinBorder = 0;
    Color backdropColor{};
    Color parchmentColor{};
    Color woodBorderColor{};
    Color localRingColor{};
    Color inkColor{};
    std::vector<WorldMapExteriorConfig> exteriors;

private:
    void loadPanel(const tomlcfg::TomlDocument& panel);
    void loadArrows(const tomlcfg::TomlDocument& arrows);
    void loadPins(const tomlcfg::TomlDocument& pins);
    void loadText(const tomlcfg::TomlDocument& text);
    void loadColors(const tomlcfg::TomlDocument& colors);
    void loadExteriors(const tomlcfg::TomlDocument& doc);

    Color readColor(const tomlcfg::TomlDocument& colors, const std::string& key) const;
    int readPositive(const tomlcfg::TomlDocument& doc, const std::string& key) const;
    int readNonNegative(const tomlcfg::TomlDocument& doc, const std::string& key) const;
    double readFraction(const tomlcfg::TomlDocument& doc, const std::string& key) const;
    std::string readNonEmpty(const tomlcfg::TomlDocument& doc, const std::string& key) const;
    tomlcfg::TomlDocument requireTable(const tomlcfg::TomlDocument& doc,
                                       const std::string& key) const;
};

}  // namespace world
