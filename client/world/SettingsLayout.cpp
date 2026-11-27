#include "world/SettingsLayout.h"

#include <array>
#include <cstddef>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/WorldConstants.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;

// Nombre de la categoría en el TOML -> categoría. Sumar una categoría =
// una fila acá (y su valor en SoundCategory).
struct CategoryName {
    const char* name;
    SoundCategory category;
};

constexpr std::array<CategoryName, 2> kCategoryNames = {{
        {"sfx", SoundCategory::Sfx},
        {"music", SoundCategory::Music},
}};

}  // namespace

SettingsLayout::SettingsLayout(const std::string& path) {
    using Kind = ConfigErrorKind;
    try {
        const TomlDocument doc(path, TomlDocument::Source::File);
        const std::array<int, 2> gridData = doc.getIntArray<2>("settings.grid");
        grid = {gridData[0], gridData[1]};
        const std::array<int, 4> color = doc.getIntArray<4>("settings.text_color");
        textColor = Color{color[0], color[1], color[2], color[3]};
        title = doc.getString("settings.title");
        titleSize = doc.getInt("settings.title_size");
        subtitleSize = doc.getInt("settings.subtitle_size");
        labelSize = doc.getInt("settings.label_size");
        const std::array<int, 2> pad = doc.getIntArray<2>("settings.padding");
        padding = {pad[0], pad[1]};
        gap = doc.getInt("settings.gap");
        columnGap = doc.getInt("settings.column_gap");
        titleGap = doc.getInt("settings.title_gap");
        sectionGap = doc.getInt("settings.section_gap");
        titleHeight = doc.getInt("settings.title_height");
        subtitleHeight = doc.getInt("settings.subtitle_height");
        rowHeight = doc.getInt("settings.row_height");
        labelWidth = doc.getInt("settings.label_width");

        tab.sheet = doc.getString("settings.tab.sheet");
        const std::array<int, 2> tabCell = doc.getIntArray<2>("settings.tab.cell");
        tab.cell = {tabCell[0], tabCell[1]};
        const std::array<int, 2> tabAt = doc.getIntArray<2>("settings.tab.at");
        tab.at = SheetCell{tabAt[0], tabAt[1]};
        const std::array<int, 2> tabSize = doc.getIntArray<2>("settings.tab.size");
        tab.size = {tabSize[0], tabSize[1]};
        const std::array<int, 2> tabOffset = doc.getIntArray<2>("settings.tab.offset");
        tab.offset = {tabOffset[0], tabOffset[1]};

        multiplayerTitle = doc.getString("settings.multiplayer.title");
        qrLabel = doc.getString("settings.multiplayer.qr_label");
        qrAreaWidth = doc.getInt("settings.multiplayer.area_width");
        const std::array<int, 2> button = doc.getIntArray<2>("settings.multiplayer.button_size");
        qrButtonSize = {button[0], button[1]};
        const std::array<int, 2> buttonOffset =
                doc.getIntArray<2>("settings.multiplayer.button_offset");
        qrButtonOffset = {buttonOffset[0], buttonOffset[1]};
        audioTitle = doc.getString("settings.audio.title");

        leaveLabel = doc.getString("settings.leave.label");
        leaveLabelWidth = doc.getInt("settings.leave.label_width");
        const std::array<int, 2> leaveButton = doc.getIntArray<2>("settings.leave.button_size");
        leaveButtonSize = {leaveButton[0], leaveButton[1]};
        const std::array<int, 2> leaveOffset = doc.getIntArray<2>("settings.leave.button_offset");
        leaveButtonOffset = {leaveOffset[0], leaveOffset[1]};

        slider.lineThickness = doc.getInt("settings.slider.line_thickness");
        const std::array<int, 4> line = doc.getIntArray<4>("settings.slider.line_color");
        slider.lineColor = Color{line[0], line[1], line[2], line[3]};
        knobImage = doc.getString("settings.slider.knob");
        const std::array<int, 2> knob = doc.getIntArray<2>("settings.slider.knob_size");
        slider.knob = {knob[0], knob[1]};

        icon.image = doc.getString("settings.icon.image");
        icon.mutedImage = doc.getString("settings.icon.muted_image");
        const std::array<int, 2> iconCell = doc.getIntArray<2>("settings.icon.cell");
        icon.cell = {iconCell[0], iconCell[1]};
        const std::array<int, 2> iconSize = doc.getIntArray<2>("settings.icon.size");
        icon.size = {iconSize[0], iconSize[1]};

        percent.width = doc.getInt("settings.percent.width");
        percent.sign = doc.getString("settings.percent.sign");
        percent.signWidth = doc.getInt("settings.percent.sign_width");
        percent.digitSize = doc.getInt("settings.percent.digit_size");
        percent.digitStep = doc.getInt("settings.percent.digit_step");

        for (const TomlDocument& entry: doc.getTableArray("settings.volume")) {
            VolumeRow row;
            const std::string name = entry.getString("category");
            bool known = false;
            for (const CategoryName& candidate: kCategoryNames) {
                if (name == candidate.name) {
                    row.category = candidate.category;
                    known = true;
                }
            }
            if (!known) {
                throw ConfigError(Kind::InvalidValue, "settings.volume.category");
            }
            row.label = entry.getString("label");
            row.defaultPercent = entry.getInt("default");
            if (row.defaultPercent < kMinVolumePercent || row.defaultPercent > kMaxVolumePercent) {
                throw ConfigError(Kind::InvalidValue, "settings.volume.default");
            }
            volumeRows.push_back(row);
        }
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
    if (grid.columns <= 0 || grid.rows <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.grid");
    }
    if (titleSize <= 0 || subtitleSize <= 0 || labelSize <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.label_size");
    }
    if (rowHeight <= 0 || labelWidth <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.row_height");
    }
    if (tab.cell.width <= 0 || tab.cell.height <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.tab.cell");
    }
    if (slider.lineThickness <= 0 || slider.knob.width <= 0 || slider.knob.height <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.slider");
    }
    if (icon.cell.width <= 0 || icon.cell.height <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.icon.cell");
    }
    if (percent.digitSize <= 0 || percent.digitStep <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.percent.digit_size");
    }
    if (leaveLabelWidth <= 0 || leaveButtonSize.width <= 0 || leaveButtonSize.height <= 0) {
        throw ConfigError(Kind::InvalidValue, "settings.leave");
    }
    if (volumeRows.empty()) {
        throw ConfigError(Kind::InvalidValue, "settings.volume");
    }
}

std::map<SoundCategory, int> SettingsLayout::defaultVolumes() const {
    std::map<SoundCategory, int> volumes;
    for (const VolumeRow& row: volumeRows) {
        volumes[row.category] = row.defaultPercent;
    }
    return volumes;
}

SettingsBoxes SettingsLayout::boxes(const PixelBox& panel) const {
    const int left = panel.x + padding.width;
    const int innerWidth = panel.width - 2 * padding.width;
    const int right = left + innerWidth;
    int y = panel.y + padding.height;
    // Los bloques se apilan de arriba hacia abajo con `gap` entre ellos.
    const auto nextBand = [&](const int height) {
        const PixelBox band{left, y, innerWidth, height};
        y += height + gap;
        return band;
    };

    SettingsBoxes result;
    result.tab = PixelBox{panel.x + tab.offset.width, panel.y - tab.size.height + tab.offset.height,
                          tab.size.width, tab.size.height};
    result.title = nextBand(titleHeight);
    y += titleGap;
    result.multiplayerTitle = nextBand(subtitleHeight);

    const PixelBox qrBand = nextBand(rowHeight);
    result.qrLabel = PixelBox{qrBand.x, qrBand.y, labelWidth, qrBand.height};
    // Etiqueta, espacio reservado y botón, uno a continuación del otro.
    const int areaX = qrBand.x + labelWidth + columnGap;
    result.qrArea = PixelBox{areaX, qrBand.y, qrAreaWidth, qrBand.height};
    result.qrButton = PixelBox{areaX + qrAreaWidth + columnGap + qrButtonOffset.width,
                               qrBand.y + (qrBand.height - qrButtonSize.height) / 2 +
                                       qrButtonOffset.height,
                               qrButtonSize.width, qrButtonSize.height};

    y += sectionGap;
    result.audioTitle = nextBand(subtitleHeight);
    for (const VolumeRow& row: volumeRows) {
        result.volume.push_back(volumeRowBoxes(row, nextBand(rowHeight), right));
    }

    // Abandonar: la pregunta y el botón pegado a su costado.
    y += sectionGap;
    const PixelBox leaveBand = nextBand(rowHeight);
    result.leaveLabel = PixelBox{leaveBand.x, leaveBand.y, leaveLabelWidth, leaveBand.height};
    result.leaveButton = PixelBox{leaveBand.x + leaveLabelWidth + columnGap + leaveButtonOffset.width,
                                  leaveBand.y + (leaveBand.height - leaveButtonSize.height) / 2 +
                                          leaveButtonOffset.height,
                                  leaveButtonSize.width, leaveButtonSize.height};
    return result;
}

VolumeRowBoxes SettingsLayout::volumeRowBoxes(const VolumeRow& row, const PixelBox& band,
                                              const int innerRight) const {
    VolumeRowBoxes result;
    result.category = row.category;
    result.label = PixelBox{band.x, band.y, labelWidth, band.height};
    result.percent = PixelBox{innerRight - percent.width, band.y, percent.width, band.height};
    result.icon = PixelBox{result.percent.x - columnGap - icon.size.width,
                           band.y + (band.height - icon.size.height) / 2, icon.size.width,
                           icon.size.height};
    const int trackX = band.x + labelWidth + columnGap;
    result.track = PixelBox{trackX, band.y + (band.height - slider.knob.height) / 2,
                            result.icon.x - columnGap - trackX, slider.knob.height};
    return result;
}

PercentBoxes SettingsLayout::percentBoxes(const PixelBox& percentArea,
                                          const int digitCount) const {
    PercentBoxes result;
    result.sign = PixelBox{percentArea.x + percentArea.width - percent.signWidth, percentArea.y,
                           percent.signWidth, percentArea.height};
    const int y = percentArea.y + (percentArea.height - percent.digitSize) / 2;
    for (int i = 0; i < digitCount; ++i) {
        const int fromEnd = digitCount - 1 - i;
        result.digits.push_back(PixelBox{result.sign.x - percent.digitSize - fromEnd * percent.digitStep,
                                         y, percent.digitSize, percent.digitSize});
    }
    return result;
}

bool SettingsLayout::onQrButton(const SettingsBoxes& boxes, const PixelPoint& point) const {
    return contains(boxes.qrButton, point);
}

bool SettingsLayout::onLeaveButton(const SettingsBoxes& boxes, const PixelPoint& point) const {
    return contains(boxes.leaveButton, point);
}

std::optional<SoundCategory> SettingsLayout::sliderAt(const SettingsBoxes& boxes,
                                                      const PixelPoint& point) const {
    for (const VolumeRowBoxes& row: boxes.volume) {
        if (contains(row.track, point)) {
            return row.category;
        }
    }
    return std::nullopt;
}

bool SettingsLayout::contains(const PixelBox& box, const PixelPoint& point) const {
    return point.x >= box.x && point.x < box.x + box.width && point.y >= box.y &&
           point.y < box.y + box.height;
}

}  // namespace world
