#include "gtest/gtest.h"
#include "world/ConfigError.h"
#include "world/ObtainedBannerCatalog.h"
#include "world/ObtainedBannerConfigLoader.h"

namespace {

const std::string kBannerToml = R"(
size_factor    = 2.0
icon_margin    = 8
duration_ticks = 120
max_visible    = 4
stack_gap      = 4

[frame]
sheet        = "Players/main/tools/MenuTiles.png"
cell         = [16, 16]
top_left     = [0, 16]
top_right    = [3, 16]
bottom_left  = [0, 19]
bottom_right = [3, 19]
top          = [1, 16]
bottom       = [1, 19]
left         = [0, 17]
right        = [3, 17]
fill         = [1, 17]

[icons.silver]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [6, 1]

[icons.gold]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [6, 7]
)";

world::ObtainedBannerCatalog loadString(const std::string& toml) {
    world::ObtainedBannerConfigLoader loader;
    return loader.loadFromString(toml);
}

TEST(ObtainedBannerConfigLoaderTest, LoadsLayoutParameters) {
    const world::ObtainedBannerCatalog catalog = loadString(kBannerToml);
    EXPECT_DOUBLE_EQ(catalog.sizeFactor, 2.0);
    EXPECT_EQ(catalog.iconMargin, 8);
    EXPECT_EQ(catalog.durationTicks, 120);
    EXPECT_EQ(catalog.maxVisible, 4);
    EXPECT_EQ(catalog.stackGap, 4);
}

TEST(ObtainedBannerConfigLoaderTest, IconOffsetsDefaultToZero) {
    const world::ObtainedBannerCatalog catalog = loadString(kBannerToml);
    EXPECT_EQ(catalog.iconOffsetX, 0);
    EXPECT_EQ(catalog.iconOffsetY, 0);
}

TEST(ObtainedBannerConfigLoaderTest, LoadsIconOffsets) {
    const std::string toml = R"(
icon_offset_x = 2
icon_offset_y = -3
[icons.silver]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [6, 1]
)";
    const world::ObtainedBannerCatalog catalog = loadString(toml);
    EXPECT_EQ(catalog.iconOffsetX, 2);
    EXPECT_EQ(catalog.iconOffsetY, -3);
}

TEST(ObtainedBannerConfigLoaderTest, LoadsFrameCells) {
    const world::ObtainedBannerCatalog catalog = loadString(kBannerToml);
    EXPECT_EQ(catalog.frame.sheetImage, "Players/main/tools/MenuTiles.png");
    EXPECT_EQ(catalog.frame.cellWidth, 16);
    EXPECT_EQ(catalog.frame.cellHeight, 16);
    EXPECT_EQ(catalog.frame.topLeft.col, 0);
    EXPECT_EQ(catalog.frame.topLeft.row, 16);
    EXPECT_EQ(catalog.frame.bottomRight.col, 3);
    EXPECT_EQ(catalog.frame.bottomRight.row, 19);
    EXPECT_EQ(catalog.frame.fill.col, 1);
    EXPECT_EQ(catalog.frame.fill.row, 17);
}

TEST(ObtainedBannerConfigLoaderTest, LoadsDeclaredIcons) {
    const world::ObtainedBannerCatalog catalog = loadString(kBannerToml);
    ASSERT_NE(catalog.find("silver"), nullptr);
    ASSERT_NE(catalog.find("gold"), nullptr);
    const world::SpriteVisual& gold = *catalog.find("gold");
    EXPECT_EQ(gold.sheetImage, "Cavern/ore.png");
    EXPECT_EQ(gold.cellWidth, 16);
    EXPECT_EQ(gold.cellHeight, 16);
    EXPECT_EQ(gold.col, 6);
    EXPECT_EQ(gold.row, 7);
}

TEST(ObtainedBannerConfigLoaderTest, UndeclaredIconIsNotInCatalog) {
    const world::ObtainedBannerCatalog catalog = loadString(kBannerToml);
    EXPECT_EQ(catalog.find("coal"), nullptr);
    EXPECT_EQ(catalog.find("lapis"), nullptr);
}

TEST(ObtainedBannerConfigLoaderTest, DefaultsWhenLayoutOmitted) {
    const std::string toml = R"(
[icons.silver]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [6, 1]
)";
    const world::ObtainedBannerCatalog catalog = loadString(toml);
    EXPECT_DOUBLE_EQ(catalog.sizeFactor, 2.0);
    EXPECT_EQ(catalog.iconMargin, 8);
    EXPECT_EQ(catalog.durationTicks, 120);
    EXPECT_EQ(catalog.maxVisible, 4);
    EXPECT_EQ(catalog.stackGap, 4);
}

TEST(ObtainedBannerConfigLoaderTest, EmptyTomlGivesEmptyIcons) {
    const world::ObtainedBannerCatalog catalog = loadString("");
    EXPECT_EQ(catalog.find("silver"), nullptr);
    EXPECT_EQ(catalog.find("gold"), nullptr);
}

TEST(ObtainedBannerConfigLoaderTest, NegativeIconCellIsInvalidValue) {
    const std::string toml = R"(
[icons.silver]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [-1, 1]
)";
    try {
        loadString(toml);
        FAIL() << "se esperaba ConfigError";
    } catch (const world::ConfigError& error) {
        EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
    }
}

TEST(ObtainedBannerConfigLoaderTest, NonPositiveMaxVisibleIsInvalidValue) {
    const std::string toml = R"(
max_visible = 0
[icons.silver]
sheet = "Cavern/ore.png"
cell  = [16, 16]
icon  = [6, 1]
)";
    try {
        loadString(toml);
        FAIL() << "se esperaba ConfigError";
    } catch (const world::ConfigError& error) {
        EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
    }
}

}  // namespace
