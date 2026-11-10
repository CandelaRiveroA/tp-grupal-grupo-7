#include "gtest/gtest.h"

#include "world/ConfigError.h"
#include "world/ResourceCatalog.h"
#include "world/ResourceConfigLoader.h"

namespace {

// cpplint pide C string para constantes globales; loadString convierte.
const char kMineralsToml[] = R"(
[resources.silver]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [0, 0, 2, 2]
cracked = [2, 0, 2, 2]
broken  = [4, 0, 2, 2]

[resources.gold]
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
	EXPECT_NE(catalog.find("silver", world::ResourceState::Intact), nullptr);
	EXPECT_NE(catalog.find("gold", world::ResourceState::Broken), nullptr);
}

TEST(ResourceConfigLoaderTest, UndeclaredResourceIsNotInCatalog) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_EQ(catalog.find("coal", world::ResourceState::Intact), nullptr);
}

// Vocabulario ABIERTO: cualquier id de [resources.*] carga sin tocar
// código (así se agregan recursos nuevos, ej. rocas destructibles).
TEST(ResourceConfigLoaderTest, LoadsAnyDeclaredId) {
	const std::string toml = R"(
[resources.boulder]
sheet   = "Cavern/Mines7_dangerous.png"
cell    = [16, 16]
intact  = [0, 0, 2, 2]
cracked = [0, 0, 2, 2]
broken  = [0, 0, 2, 2]
)";
	const world::ResourceCatalog catalog = loadString(toml);
	EXPECT_NE(catalog.find("boulder", world::ResourceState::Intact), nullptr);
}

TEST(ResourceConfigLoaderTest, LoadsAllThreeStates) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	EXPECT_NE(catalog.find("silver", world::ResourceState::Intact), nullptr);
	EXPECT_NE(catalog.find("silver", world::ResourceState::Cracked), nullptr);
	EXPECT_NE(catalog.find("silver", world::ResourceState::Broken), nullptr);
}

TEST(ResourceConfigLoaderTest, LoadsVisualBlock) {
	const world::ResourceCatalog catalog = loadString(kMineralsToml);
	const world::ResourceVisual* cracked =
		catalog.find("silver", world::ResourceState::Cracked);
	ASSERT_NE(cracked, nullptr);
	EXPECT_EQ(cracked->sprite.sheetImage, "Cavern/ore.png");
	EXPECT_EQ(cracked->sprite.cellWidth, 16);
	EXPECT_EQ(cracked->sprite.cellHeight, 16);
	EXPECT_EQ(cracked->sprite.col, 2);
	EXPECT_EQ(cracked->sprite.row, 0);
	EXPECT_EQ(cracked->cols, 2);
	EXPECT_EQ(cracked->rows, 2);
}

TEST(ResourceConfigLoaderTest, EmptyTomlGivesEmptyCatalog) {
	const world::ResourceCatalog catalog = loadString("");
	EXPECT_EQ(catalog.find("silver", world::ResourceState::Intact), nullptr);
}

TEST(ResourceConfigLoaderTest, NegativeCellIsInvalidValue) {
	const std::string toml = R"(
[resources.silver]
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
		EXPECT_EQ(error.path(), "resources.silver.intact");
	}
}

// Los tres estados son obligatorios: el error lleva el path crudo (§3).
TEST(ResourceConfigLoaderTest, MissingStateIsConfigError) {
	const std::string toml = R"(
[resources.silver]
sheet   = "Cavern/ore.png"
cell    = [16, 16]
intact  = [0, 0, 2, 2]
cracked = [2, 0, 2, 2]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::MissingStructure);
		EXPECT_EQ(error.path(), "resources.silver.broken");
	}
}

TEST(ResourceConfigLoaderTest, LoadsGlobalOffsetAndScale) {
	const std::string toml = R"(
offset_y = 8
scale = 1.3
[resources.silver]
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
