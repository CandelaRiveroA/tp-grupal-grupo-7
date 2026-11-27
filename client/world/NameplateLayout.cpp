#include "world/NameplateLayout.h"

#include <array>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

namespace {

constexpr const char* kFont = "nameplate.font";
constexpr const char* kFontSize = "nameplate.font_size";
constexpr const char* kBold = "nameplate.bold";
constexpr const char* kGap = "nameplate.gap";
constexpr const char* kBoxHeight = "nameplate.box_height";
constexpr const char* kDayColor = "nameplate.day_color";
constexpr const char* kNightColor = "nameplate.night_color";
constexpr const char* kNightFromHour = "nameplate.night_from_hour";

}  // namespace

NameplateLayout::NameplateLayout(const std::string& path) {
    using Kind = ConfigErrorKind;
    try {
        const tomlcfg::TomlDocument doc(path, tomlcfg::TomlDocument::Source::File);
        if (doc.hasKey(kFont)) {
            fontFace_ = doc.getString(kFont);
        }
        fontSize_ = doc.getInt(kFontSize);
        if (doc.hasKey(kBold)) {
            bold_ = doc.getBool(kBold);
        }
        gap_ = doc.getInt(kGap);
        boxHeight_ = doc.getInt(kBoxHeight);
        const std::array<int, 4> day = doc.getIntArray<4>(kDayColor);
        dayColor_ = Color{day[0], day[1], day[2], day[3]};
        const std::array<int, 4> night = doc.getIntArray<4>(kNightColor);
        nightColor_ = Color{night[0], night[1], night[2], night[3]};
        nightFromHour_ = doc.getInt(kNightFromHour);
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
    if (fontFace_.empty()) {
        throw ConfigError(Kind::InvalidValue, kFont);
    }
    if (fontSize_ <= 0) {
        throw ConfigError(Kind::InvalidValue, kFontSize);
    }
    if (gap_ < 0) {
        throw ConfigError(Kind::InvalidValue, kGap);
    }
    if (boxHeight_ <= 0) {
        throw ConfigError(Kind::InvalidValue, kBoxHeight);
    }
}

PixelRect NameplateLayout::boxUnder(const PixelRect& drawn, const int textWidth) const {
    const int feetY = drawn.y + drawn.height;
    const int centerX = drawn.x + drawn.width / 2;
    return PixelRect{centerX - textWidth / 2, feetY + gap_, textWidth, boxHeight_};
}

const Color& NameplateLayout::colorAt(const int hour) const {
    return hour >= nightFromHour_ ? nightColor_ : dayColor_;
}

const std::string& NameplateLayout::fontFace() const { return fontFace_; }

int NameplateLayout::fontSize() const { return fontSize_; }

bool NameplateLayout::bold() const { return bold_; }

}  // namespace world
