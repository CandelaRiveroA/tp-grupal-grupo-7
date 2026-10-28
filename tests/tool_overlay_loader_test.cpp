#include "gtest/gtest.h"

#include "world/ConfigError.h"
#include "world/ToolOverlay.h"
#include "world/ToolOverlayLoader.h"

namespace {

const std::string kMinimalPickaxeBack = R"(
[pickaxe.back]
sheet       = "Players/main/tools/Tools.png"
cell        = [16, 16]
frame_ticks = 6
frames = [
  { anchor = "top", layers = [ [3, 5, 1, 2] ] },
  { layers = [] },
  { layers = [] },
  { layers = [] },
  { layers = [] },
]
)";

world::ToolOverlayCatalog loadString(const std::string& toml) {
	world::ToolOverlayLoader loader;
	return loader.loadFromString(toml);
}

TEST(ToolOverlayLoaderTest, LoadsPickaxeBack) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	ASSERT_TRUE(catalog.has(world::ItemKind::Pickaxe,
				world::Direction::Back));
}

TEST(ToolOverlayLoaderTest, LoadsPerAnchorOffsets) {
	const std::string toml = R"(
[offset]
top    = 10
body   = 4
bottom = 0

[pickaxe.back]
sheet       = "Tools.png"
cell        = [16, 16]
frame_ticks = 6
frames = [
  { anchor = "top", layers = [ [3, 5, 1, 2] ] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Top), 10);
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Body), 4);
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Bottom), 0);
}

TEST(ToolOverlayLoaderTest, MissingOffsetDefaultsToZero) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	// Sin tabla [offset] → todos los anclajes dan 0.
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Top), 0);
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Body), 0);
	EXPECT_EQ(catalog.offsetFor(world::FrameAnchor::Bottom), 0);
}

TEST(ToolOverlayLoaderTest, LoadsSheetAndCell) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.sheetImage, "Players/main/tools/Tools.png");
	EXPECT_EQ(action.cellWidth, 16);
	EXPECT_EQ(action.cellHeight, 16);
}

TEST(ToolOverlayLoaderTest, LoadsFrameTicks) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.frameTicks, 6);
}

TEST(ToolOverlayLoaderTest, LoadsFrameCount) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.frames.size(), 5u);
}

TEST(ToolOverlayLoaderTest, LoadsFrameCells) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	ASSERT_FALSE(action.frames.empty());
	ASSERT_FALSE(action.frames[0].layers.empty());
	EXPECT_EQ(action.frames[0].layers[0].frame.col, 3);
	EXPECT_EQ(action.frames[0].layers[0].frame.row, 5);
	EXPECT_EQ(action.frames[0].layers[0].cols, 1);
	EXPECT_EQ(action.frames[0].layers[0].rows, 2);
}

TEST(ToolOverlayLoaderTest, MissingItemIsNotInCatalog) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	EXPECT_FALSE(catalog.has(world::ItemKind::Shovel,
				 world::Direction::Back));
}

TEST(ToolOverlayLoaderTest, MissingDirectionIsNotInCatalog) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	EXPECT_FALSE(catalog.has(world::ItemKind::Pickaxe,
				 world::Direction::Front));
}

TEST(ToolOverlayLoaderTest, EmptyTomlGivesEmptyCatalog) {
	const world::ToolOverlayCatalog catalog = loadString("");
	EXPECT_FALSE(catalog.has(world::ItemKind::Pickaxe,
				 world::Direction::Back));
}

TEST(ToolOverlayLoaderTest, EmptyFrameHasNoLayers) {
	const std::string toml = R"(
[pickaxe.back]
sheet       = "Tools.png"
cell        = [16, 16]
frame_ticks = 6
frames = [
  { anchor = "top", layers = [ [3, 5, 1, 2] ] },
  { layers = [] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	ASSERT_EQ(action.frames.size(), 2u);
	EXPECT_FALSE(action.frames[0].layers.empty());
	EXPECT_TRUE(action.frames[1].layers.empty());
}

TEST(ToolOverlayLoaderTest, FrameAnchorDefaultsToTop) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	// El frame vacío no declaró anchor → default Top.
	EXPECT_EQ(action.frames[1].anchor, world::FrameAnchor::Top);
}

TEST(ToolOverlayLoaderTest, LoadsFrameAnchors) {
	const std::string toml = R"(
[pickaxe.front]
sheet       = "Tools.png"
cell        = [16, 16]
frame_ticks = 6
frames = [
  { anchor = "top",    layers = [ [0, 5, 1, 2] ] },
  { anchor = "body",   layers = [ [3, 5, 1, 2] ] },
  { anchor = "bottom", layers = [ [1, 5, 1, 2] ] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	const world::ToolOverlayAction& action =
		catalog.actionFor(world::ItemKind::Pickaxe,
				  world::Direction::Front);
	ASSERT_EQ(action.frames.size(), 3u);
	EXPECT_EQ(action.frames[0].anchor, world::FrameAnchor::Top);
	EXPECT_EQ(action.frames[1].anchor, world::FrameAnchor::Body);
	EXPECT_EQ(action.frames[2].anchor, world::FrameAnchor::Bottom);
}

TEST(ToolOverlayLoaderTest, InvalidAnchorIsInvalidValue) {
	const std::string toml = R"(
[pickaxe.back]
sheet       = "Tools.png"
cell        = [16, 16]
frame_ticks = 6
frames = [
  { anchor = "sideways", layers = [ [3, 5, 1, 2] ] },
]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

}
