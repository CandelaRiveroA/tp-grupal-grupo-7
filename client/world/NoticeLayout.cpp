#include "world/NoticeLayout.h"

#include <array>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

NoticeLayout::NoticeLayout(const std::string& path) {
    using Kind = ConfigErrorKind;
    try {
        const tomlcfg::TomlDocument doc(path, tomlcfg::TomlDocument::Source::File);
        sheetImage = doc.getString("notice.sheet");
        const std::array<int, 2> cell = doc.getIntArray<2>("notice.cell");
        cellWidth = cell[0];
        cellHeight = cell[1];
        border = doc.getInt("notice.border");
        const std::array<int, 2> margin = doc.getIntArray<2>("notice.margin");
        marginX = margin[0];
        marginY = margin[1];
        const std::array<int, 2> padding = doc.getIntArray<2>("notice.padding");
        padX = padding[0];
        padY = padding[1];
        fontSize = doc.getInt("notice.font_size");
        if (doc.hasKey("notice.font")) {
            fontFace = doc.getString("notice.font");
        }
        const std::array<int, 4> color = doc.getIntArray<4>("notice.color");
        textColor = Color{color[0], color[1], color[2], color[3]};
        durationTicks = doc.getInt("notice.duration_ticks");

        const std::optional<Anchor> parsed =
                AnchorLayout().fromName(doc.getString("notice.anchor"));
        if (!parsed.has_value()) {
            throw ConfigError(Kind::InvalidValue, "notice.anchor");
        }
        anchor = *parsed;

        for (const tomlcfg::TomlDocument& entry: doc.getTableArray("notice.entry")) {
            texts[entry.getString("id")] = entry.getString("text");
        }
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
    if (sheetImage.empty()) {
        throw ConfigError(Kind::InvalidValue, "notice.sheet");
    }
    if (cellWidth <= 0 || cellHeight <= 0) {
        throw ConfigError(Kind::InvalidValue, "notice.cell");
    }
    if (fontSize <= 0) {
        throw ConfigError(Kind::InvalidValue, "notice.font_size");
    }
    if (durationTicks <= 0) {
        throw ConfigError(Kind::InvalidValue, "notice.duration_ticks");
    }
}

const std::string* NoticeLayout::textFor(const std::string& id) const {
    const auto it = texts.find(id);
    return it == texts.end() ? nullptr : &it->second;
}

PixelBox NoticeLayout::boxFor(int textWidth, int windowWidth, int windowHeight) const {
    const int width = textWidth + 2 * padX;
    const int height = fontSize + 2 * padY;
    return AnchorLayout().box(anchor, marginX, marginY, width, height, windowWidth,
                              windowHeight);
}

}  // namespace world
