#include "gtest/gtest.h"

#include "world/ConfigError.h"
#include "world/ResourceCatalog.h"
#include "world/ResourceConfigLoader.h"

namespace {

const std::string kMineralsToml = R"(
[silver]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [0, 0, 2, 2]
cracked = [2, 0, 2, 2]
broken  = [4, 0, 2, 2]

[gold]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [0, 6, 2, 2]
cracked = [2, 6, 2, 2]
broken  = [4, 6, 2, 2]
)";

world::ResourceCatalog loadString(const std::string& toml) {
	world::ResourceConfigLoader loader;
	return loader.loadFromString(toml);
}

TEST(ResourceConfigLoaderTest, LoadsDeclaredResources) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_TRUE(catalog.has(world::ResourceKind::Silver,
				world::ResourceState::Intact));
	EXPECT_TRUE(catalog.has(world::ResourceKind::Gold,
				world::ResourceState::Broken));
}

TEST(ResourceConfigLoaderTest, UndeclaredResourceIsNotInCatalog) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_FALSE(catalog.has(world::ResourceKind::Coal,
				 world::ResourceState::Intact));
}

TEST(ResourceConfigLoaderTest, LoadsAllThreeStates) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_TRUE(catalog.has(world::ResourceKind::Silver,
				world::ResourceState::Intact));
	EXPECT_TRUE(catalog.has(world::ResourceKind::Silver,
				world::ResourceState::Cracked));
	EXPECT_TRUE(catalog.has(world::ResourceKind::Silver,
				world::ResourceState::Broken));
}

TEST(ResourceConfigLoaderTest, LoadsVisualBlock) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	const world::ResourceVisual& cracked = catalog.visualFor(
		world::ResourceKind::Silver, world::ResourceState::Cracked);
	EXPECT_EQ(cracked.sheetImage, "Cavern/ore.png");
	EXPECT_EQ(cracked.cellWidth, 16);
	EXPECT_EQ(cracked.cellHeight, 16);
	EXPECT_EQ(cracked.col, 2);
	EXPECT_EQ(cracked.row, 0);
	EXPECT_EQ(cracked.cols, 2);
	EXPECT_EQ(cracked.rows, 2);
}

TEST(ResourceConfigLoaderTest, EmptyTomlGivesEmptyCatalog) {
	const world::ResourceCatalog catalog = loadString("");
	EXPECT_FALSE(catalog.has(world::ResourceKind::Silver,
				 world::ResourceState::Intact));
}

TEST(ResourceConfigLoaderTest, NegativeCellIsInvalidValue) {
	const std::string toml = R"(
[silver]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [-1, 0, 2, 2]
cracked = [2, 0, 2, 2]
broken  = [4, 0, 2, 2]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

TEST(ResourceConfigLoaderTest, LoadsGlobalOffsetAndScale) {
	const std::string toml = R"(
offset_y = 8
scale = 1.3
[silver]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [0, 0, 2, 2]
cracked = [2, 0, 2, 2]
broken  = [4, 0, 2, 2]
)";
	const world::ResourceCatalog catalog = loadString(toml);
	EXPECT_EQ(catalog.offsetY, 8);
	EXPECT_DOUBLE_EQ(catalog.scale, 1.3);
}

TEST(ResourceConfigLoaderTest, OffsetAndScaleHaveDefaults) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_EQ(catalog.offsetY, 0);
	EXPECT_DOUBLE_EQ(catalog.scale, 1.0);
}

}
