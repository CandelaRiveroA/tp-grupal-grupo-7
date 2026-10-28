#include <string>

#include "gtest/gtest.h"
#include "inventory_fixtures.h"
#include "world/ConfigError.h"
#include "world/InventoryConfigLoader.h"

namespace {

using inventory_fixtures::kBarToml;
using inventory_fixtures::loadBar;
using inventory_fixtures::loadBarWithFrame;
using inventory_fixtures::loadString;


TEST(InventoryConfigLoaderTest, LoadsSheet) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_EQ(bar.sheetImage, "Players/main/tools/MenuTiles.png");
	EXPECT_EQ(bar.cellWidth, 16);
	EXPECT_EQ(bar.cellHeight, 16);
}

TEST(InventoryConfigLoaderTest, LoadsSlotCell) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_EQ(bar.slotCol, 4);
	EXPECT_EQ(bar.slotRow, 8);
}

TEST(InventoryConfigLoaderTest, LoadsBar) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_EQ(bar.count, 9);
	EXPECT_EQ(bar.scale, 3);
	EXPECT_EQ(bar.gap, 0);
	EXPECT_EQ(bar.margin, 16);
	EXPECT_EQ(bar.anchor, world::InventoryAnchor::BottomCenter);
}

TEST(InventoryConfigLoaderTest, DefaultsUiScaleToOne) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_DOUBLE_EQ(bar.uiScale, 1.0);
}

TEST(InventoryConfigLoaderTest, LoadsUiScale) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
ui_scale = 0.8
)";
	const world::InventoryLayout bar = loadString(toml);
	EXPECT_DOUBLE_EQ(bar.uiScale, 0.8);
}

TEST(InventoryConfigLoaderTest, ZeroUiScaleIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
ui_scale = 0.0
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, DerivedSizes) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_EQ(bar.slotDrawWidth(), 48);
	EXPECT_EQ(bar.slotDrawHeight(), 48);
	EXPECT_EQ(bar.barWidth(), 432);
}


TEST(InventoryConfigLoaderTest, InvalidTomlThrows) {
	EXPECT_THROW(loadString("not = = toml"),
		     world::ConfigError);
}

TEST(InventoryConfigLoaderTest, MissingSheetIsMissingStructure) {
	const std::string toml = R"(
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 3
gap = 0
anchor = "bottom_center"
margin = 16
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::MissingStructure);
	}
}

TEST(InventoryConfigLoaderTest, ZeroCountIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 0
scale = 3
gap = 0
anchor = "bottom_center"
margin = 16
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, UnknownAnchorIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 3
gap = 0
anchor = "top_left"
margin = 16
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, WrongTypeCellIsWrongType) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = "no-es-una-lista"
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 3
gap = 0
anchor = "bottom_center"
margin = 16
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::WrongType);
	}
}


TEST(InventoryConfigLoaderTest, NoFrameSectionLeavesFrameDisabled) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_FALSE(bar.frame.enabled);
	EXPECT_EQ(bar.borderThickness(), 0);
}

TEST(InventoryConfigLoaderTest, LoadsItemCatalog) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
[items.shovel]
sheet = "Players/main/tools/Tools.png"
cell  = [16, 16]
at    = [5, 2]
[items.pickaxe]
sheet = "Players/main/tools/Tools.png"
cell  = [16, 16]
at    = [5, 6]
[items.watering_can]
sheet = "Players/main/tools/Tools.png"
cell  = [16, 16]
at    = [3, 14]
)";
	const world::InventoryLayout bar = loadString(toml);
	ASSERT_TRUE(bar.items.has(world::ItemKind::Shovel));
	ASSERT_TRUE(bar.items.has(world::ItemKind::Pickaxe));
	ASSERT_TRUE(bar.items.has(world::ItemKind::WateringCan));
	const world::ItemVisual& shovel =
		bar.items.visualFor(world::ItemKind::Shovel);
	EXPECT_EQ(shovel.sheetImage, "Players/main/tools/Tools.png");
	EXPECT_EQ(shovel.col, 5);
	EXPECT_EQ(shovel.row, 2);
	EXPECT_EQ(shovel.cellWidth, 16);
	const world::ItemVisual& can =
		bar.items.visualFor(world::ItemKind::WateringCan);
	EXPECT_EQ(can.col, 3);
	EXPECT_EQ(can.row, 14);
}

TEST(InventoryConfigLoaderTest, NoItemsSectionLeavesCatalogEmpty) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_FALSE(bar.items.has(world::ItemKind::Shovel));
}

namespace {

const char* const kItemBarToml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
)";

}

TEST(InventoryConfigLoaderTest, LoadsEggVisualWithDrawnSize) {
	const std::string toml = std::string(kItemBarToml) + R"(
[items.egg]
sheet = "Farm/egg.png"
cell  = [10, 11]
at    = [0, 0]
drawn = [15, 16]
)";
	const world::InventoryLayout bar = loadString(toml);
	ASSERT_TRUE(bar.items.has(world::ItemKind::Egg));

	const world::ItemVisual& egg =
		bar.items.visualFor(world::ItemKind::Egg);
	EXPECT_EQ(egg.sheetImage, "Farm/egg.png");
	EXPECT_EQ(egg.cellWidth, 10);
	EXPECT_EQ(egg.cellHeight, 11);
	EXPECT_EQ(egg.col, 0);
	EXPECT_EQ(egg.row, 0);
	EXPECT_EQ(egg.drawnWidth, 15);
	EXPECT_EQ(egg.drawnHeight, 16);
}

TEST(InventoryConfigLoaderTest, ItemWithoutDrawnDefaultsToNativeSize) {
	// Sin `drawn`, el tamaño en el mapa es el nativo de la celda de la
	// hoja (drawnWidth/Height en 0 = "usar cellWidth/Height").
	const std::string toml = std::string(kItemBarToml) + R"(
[items.egg]
sheet = "Farm/egg.png"
cell  = [10, 11]
at    = [0, 0]
)";
	const world::InventoryLayout bar = loadString(toml);
	ASSERT_TRUE(bar.items.has(world::ItemKind::Egg));
	const world::ItemVisual& egg =
		bar.items.visualFor(world::ItemKind::Egg);
	EXPECT_EQ(egg.drawnWidth, 0);
	EXPECT_EQ(egg.drawnHeight, 0);
}

TEST(InventoryConfigLoaderTest, ZeroDrawnIsInvalidValue) {
	const std::string toml = std::string(kItemBarToml) + R"(
[items.egg]
sheet = "Farm/egg.png"
cell  = [10, 11]
at    = [0, 0]
drawn = [0, 22]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, UnknownItemIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
[items.catapult]
sheet = "x.png"
cell  = [16, 16]
at    = [0, 0]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, LoadsFrameCells) {
	const world::InventoryLayout bar = loadBarWithFrame();
	ASSERT_TRUE(bar.frame.enabled);
	EXPECT_EQ(bar.frame.borderCells, 2);
	EXPECT_EQ(bar.frame.thickness, 16);
	EXPECT_EQ(bar.frame.topLeft.col, 1);
	EXPECT_EQ(bar.frame.topLeft.row, 1);
	EXPECT_EQ(bar.frame.topRight.col, 13);
	EXPECT_EQ(bar.frame.bottomLeft.row, 13);
	EXPECT_EQ(bar.frame.bottomRight.col, 13);
	EXPECT_EQ(bar.frame.top.col, 3);
	EXPECT_EQ(bar.frame.right.col, 13);
}

TEST(InventoryConfigLoaderTest, FrameBorderThickness) {
	const world::InventoryLayout bar = loadBarWithFrame();
	EXPECT_EQ(bar.borderThickness(), 16);
}

TEST(InventoryConfigLoaderTest, LoadsSlotFrame) {
	const world::InventoryLayout bar = loadBarWithFrame();
	ASSERT_TRUE(bar.slotFrame.enabled);
	EXPECT_EQ(bar.slotFrame.borderCells, 1);
	EXPECT_EQ(bar.slotFrame.thickness, 4);
	EXPECT_EQ(bar.slotFrame.topLeft.col, 8);
	EXPECT_EQ(bar.slotFrame.topLeft.row, 8);
	EXPECT_EQ(bar.slotFrame.bottomRight.col, 11);
	EXPECT_EQ(bar.slotFrame.bottomRight.row, 11);
	EXPECT_EQ(bar.slotFrame.top.col, 9);
	EXPECT_EQ(bar.slotFrame.left.row, 10);
}

TEST(InventoryConfigLoaderTest, NoSlotFrameLeavesItDisabled) {
	const world::InventoryLayout bar = loadBar();
	EXPECT_FALSE(bar.slotFrame.enabled);
}

TEST(InventoryConfigLoaderTest, ZeroBorderCellsIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
[frame]
border_cells = 0
thickness = 16
top_left = [1,1]
top_right = [13,1]
bottom_left = [1,13]
bottom_right = [13,13]
top = [3,1]
bottom = [3,13]
left = [1,3]
right = [13,3]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(InventoryConfigLoaderTest, ZeroThicknessIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "x.png"
cell  = [16, 16]
[slot]
cell = [4, 8]
[bar]
count = 9
scale = 4
gap = 0
anchor = "bottom_center"
margin = 16
[frame]
border_cells = 2
thickness = 0
top_left = [1,1]
top_right = [13,1]
bottom_left = [1,13]
bottom_right = [13,13]
top = [3,1]
bottom = [3,13]
left = [1,3]
right = [13,3]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

#ifdef TALLER_INVENTORY_TOML
TEST(InventoryConfigLoaderTest, LoadsRealInventoryFile) {
	world::InventoryConfigLoader loader;
	const world::InventoryLayout bar =
		loader.loadFromFile(TALLER_INVENTORY_TOML);
	EXPECT_EQ(bar.count, 9);
	EXPECT_EQ(bar.sheetImage, "Players/main/tools/MenuTiles.png");
	EXPECT_EQ(bar.slotCol, 4);
	EXPECT_EQ(bar.slotRow, 8);
	EXPECT_TRUE(bar.frame.enabled);
	EXPECT_EQ(bar.frame.borderCells, 2);
	EXPECT_TRUE(bar.slotFrame.enabled);
	EXPECT_EQ(bar.slotFrame.topLeft.col, 8);

	// El huevo de la gallina está en el catálogo (sprite único 10x11).
	ASSERT_TRUE(bar.items.has(world::ItemKind::Egg));
	const world::ItemVisual& egg =
		bar.items.visualFor(world::ItemKind::Egg);
	EXPECT_EQ(egg.sheetImage, "Farm/egg.png");
	EXPECT_EQ(egg.drawnWidth, 15);
	EXPECT_EQ(egg.drawnHeight, 16);

	// La leche de la vaca está en el catálogo (sprite único 10x12).
	ASSERT_TRUE(bar.items.has(world::ItemKind::Milk));
	const world::ItemVisual& milk =
		bar.items.visualFor(world::ItemKind::Milk);
	EXPECT_EQ(milk.sheetImage, "Farm/milk.png");
	EXPECT_EQ(milk.drawnWidth, 15);
	EXPECT_EQ(milk.drawnHeight, 18);
}
#endif

}
