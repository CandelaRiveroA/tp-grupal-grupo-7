#include "world/ObtainedBannerTomlReader.h"

#include <array>

#include "world/ConfigError.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;
using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kFrame = "frame";
constexpr const char* kSheet = "sheet";
constexpr const char* kCell = "cell";
constexpr const char* kIcon = "icon";

constexpr const char* kTopLeft = "top_left";
constexpr const char* kTopRight = "top_right";
constexpr const char* kBottomLeft = "bottom_left";
constexpr const char* kBottomRight = "bottom_right";
constexpr const char* kTop = "top";
constexpr const char* kBottom = "bottom";
constexpr const char* kLeft = "left";
constexpr const char* kRight = "right";
constexpr const char* kFill = "fill";

constexpr const char* kSizeFactor = "size_factor";
constexpr const char* kIconMargin = "icon_margin";
constexpr const char* kIconOffsetX = "icon_offset_x";
constexpr const char* kIconOffsetY = "icon_offset_y";
constexpr const char* kDurationTicks = "duration_ticks";
constexpr const char* kMaxVisible = "max_visible";
constexpr const char* kStackGap = "stack_gap";

// Iconos del cartel: subtablas [icons.<contentId>]. Cualquier contenido
// del juego puede tener su icono (vocabulario genérico: el mismo del
// inventario y los ítems del piso).
constexpr const char* kIcons = "icons";

}  // namespace

ObtainedBannerTomlReader::ObtainedBannerTomlReader(const TomlDocument& doc): doc_(doc) {}

std::string ObtainedBannerTomlReader::join(const std::string& base, const std::string& key) const {
    return base + "." + key;
}

SheetCell ObtainedBannerTomlReader::readCell(const std::string& base,
                                              const std::string& key) const {
    return SheetCell(doc_, join(base, key));
}

BannerFrame ObtainedBannerTomlReader::readFrame() const {
    BannerFrame frame;

    const std::string sheetPath = join(kFrame, kSheet);
    frame.sheetImage = doc_.getString(sheetPath);
    if (frame.sheetImage.empty()) {
        throw ConfigError(Kind::InvalidValue, sheetPath);
    }

    const std::string cellPath = join(kFrame, kCell);
    const std::array<int, 2> cell = doc_.getIntArray<2>(cellPath);
    if (cell[0] <= 0 || cell[1] <= 0) {
        throw ConfigError(Kind::InvalidValue, cellPath);
    }
    frame.cellWidth = cell[0];
    frame.cellHeight = cell[1];

    frame.topLeft = readCell(kFrame, kTopLeft);
    frame.topRight = readCell(kFrame, kTopRight);
    frame.bottomLeft = readCell(kFrame, kBottomLeft);
    frame.bottomRight = readCell(kFrame, kBottomRight);
    frame.top = readCell(kFrame, kTop);
    frame.bottom = readCell(kFrame, kBottom);
    frame.left = readCell(kFrame, kLeft);
    frame.right = readCell(kFrame, kRight);
    frame.fill = readCell(kFrame, kFill);
    return frame;
}

SpriteVisual ObtainedBannerTomlReader::readIcon(const std::string& base) const {
    return SpriteVisual(doc_, base, kIcon);
}

ObtainedBannerCatalog ObtainedBannerTomlReader::read() const {
    ObtainedBannerCatalog catalog;

    if (doc_.hasKey(kFrame)) {
        catalog.frame = readFrame();
    }

    if (doc_.hasKey(kSizeFactor)) {
        catalog.sizeFactor = doc_.getDouble(kSizeFactor);
        if (catalog.sizeFactor <= 0.0) {
            throw ConfigError(Kind::InvalidValue, kSizeFactor);
        }
    }
    if (doc_.hasKey(kIconMargin)) {
        catalog.iconMargin = doc_.getInt(kIconMargin);
        if (catalog.iconMargin < 0) {
            throw ConfigError(Kind::InvalidValue, kIconMargin);
        }
    }
    // Los offsets pueden ser negativos (mueven en cualquier sentido).
    if (doc_.hasKey(kIconOffsetX)) {
        catalog.iconOffsetX = doc_.getInt(kIconOffsetX);
    }
    if (doc_.hasKey(kIconOffsetY)) {
        catalog.iconOffsetY = doc_.getInt(kIconOffsetY);
    }
    if (doc_.hasKey(kDurationTicks)) {
        catalog.durationTicks = doc_.getInt(kDurationTicks);
        if (catalog.durationTicks <= 0) {
            throw ConfigError(Kind::InvalidValue, kDurationTicks);
        }
    }
    if (doc_.hasKey(kMaxVisible)) {
        catalog.maxVisible = doc_.getInt(kMaxVisible);
        if (catalog.maxVisible <= 0) {
            throw ConfigError(Kind::InvalidValue, kMaxVisible);
        }
    }
    if (doc_.hasKey(kStackGap)) {
        catalog.stackGap = doc_.getInt(kStackGap);
        if (catalog.stackGap < 0) {
            throw ConfigError(Kind::InvalidValue, kStackGap);
        }
    }

    // La tabla de iconos es opcional: un TOML sin [icons] no tiene
    // ningún icono (igual que los items del inventario).
    if (doc_.hasKey(kIcons)) {
        for (const std::string& name : doc_.subtableKeys(kIcons)) {
            catalog.icons.emplace(name, readIcon(join(kIcons, name)));
        }
    }
    return catalog;
}

}  // namespace world
