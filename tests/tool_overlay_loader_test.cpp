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
	ASSERT_TRUE(catalog.find(world::ItemKind::Pickaxe,
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
		*catalog.find(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.sheetImage, "Players/main/tools/Tools.png");
	EXPECT_EQ(action.cellWidth, 16);
	EXPECT_EQ(action.cellHeight, 16);
}

TEST(ToolOverlayLoaderTest, LoadsFrameTicks) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		*catalog.find(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.frameTicks, 6);
}

TEST(ToolOverlayLoaderTest, LoadsFrameCount) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		*catalog.find(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	EXPECT_EQ(action.frames.size(), 5u);
}

TEST(ToolOverlayLoaderTest, LoadsFrameCells) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		*catalog.find(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	ASSERT_FALSE(action.frames.empty());
	ASSERT_FALSE(action.frames[0].layers.empty());
	EXPECT_EQ(action.frames[0].layers[0].layer.frame.col, 3);
	EXPECT_EQ(action.frames[0].layers[0].layer.frame.row, 5);
	EXPECT_EQ(action.frames[0].layers[0].layer.cols, 1);
	EXPECT_EQ(action.frames[0].layers[0].layer.rows, 2);
}

TEST(ToolOverlayLoaderTest, MissingItemIsNotInCatalog) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	EXPECT_FALSE(catalog.find(world::ItemKind::Shovel,
				 world::Direction::Back));
}

TEST(ToolOverlayLoaderTest, MissingDirectionIsNotInCatalog) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	EXPECT_FALSE(catalog.find(world::ItemKind::Pickaxe,
				 world::Direction::Front));
}

TEST(ToolOverlayLoaderTest, EmptyTomlGivesEmptyCatalog) {
	const world::ToolOverlayCatalog catalog = loadString("");
	EXPECT_FALSE(catalog.find(world::ItemKind::Pickaxe,
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
		*catalog.find(world::ItemKind::Pickaxe,
				  world::Direction::Back);
	ASSERT_EQ(action.frames.size(), 2u);
	EXPECT_FALSE(action.frames[0].layers.empty());
	EXPECT_TRUE(action.frames[1].layers.empty());
}

TEST(ToolOverlayLoaderTest, FrameAnchorDefaultsToTop) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction& action =
		*catalog.find(world::ItemKind::Pickaxe,
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
		*catalog.find(world::ItemKind::Pickaxe,
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

TEST(ToolOverlayLoaderTest, LoadsEggForPickupAction) {
	const std::string toml = R"(
[egg.right]
sheet       = "Farm/egg.png"
cell        = [10, 11]
action      = "pickup"
frame_ticks = 12
frames = [
  { anchor = "bottom", scale = 0.5, layers = [ [0, 0, 1, 1] ] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	const world::ToolOverlayAction* action =
		catalog.find(world::ItemKind::Egg, world::Direction::Right);
	ASSERT_NE(action, nullptr);
	// Cada overlay declara con qué acción se sincroniza (dato): el
	// mismo mecanismo vale para la herramienta y para el huevo.
	EXPECT_EQ(action->action, world::ActionKind::Pickup);
	EXPECT_EQ(action->frames.size(), 1u);
}

TEST(ToolOverlayLoaderTest, ActionDefaultsToUseTool) {
	const world::ToolOverlayCatalog catalog = loadString(kMinimalPickaxeBack);
	const world::ToolOverlayAction* action =
		catalog.find(world::ItemKind::Pickaxe, world::Direction::Back);
	ASSERT_NE(action, nullptr);
	EXPECT_EQ(action->action, world::ActionKind::UseTool);
}

TEST(ToolOverlayLoaderTest, LoadsEggFrontCentered) {
	const std::string toml = R"(
[egg.front]
sheet       = "Farm/egg.png"
cell        = [10, 11]
action      = "pickup"
frame_ticks = 12
frames = [
  { anchor = "bottom", offset_x = 8, layers = [ [0, 0, 1, 1] ] },
  { anchor = "bottom", offset_x = 8, layers = [ [0, 0, 1, 1] ] },
  { anchor = "body",   offset_x = 8, layers = [ [0, 0, 1, 1] ] },
  { anchor = "body",   offset_x = 8, layers = [ [0, 0, 1, 1] ] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	const world::ToolOverlayAction* action =
		catalog.find(world::ItemKind::Egg, world::Direction::Front);
	ASSERT_NE(action, nullptr);
	EXPECT_EQ(action->action, world::ActionKind::Pickup);
	EXPECT_EQ(action->frames.size(), 4u);
	EXPECT_EQ(action->frames[0].offsetX, 8);
}

TEST(ToolOverlayLoaderTest, LoadsEggBackWithEggOnlyInLastFrame) {
	const std::string toml = R"(
[egg.back]
sheet       = "Farm/egg.png"
cell        = [10, 11]
action      = "pickup"
frame_ticks = 12
frames = [
  { layers = [] },
  { layers = [] },
  { layers = [] },
  { anchor = "body", offset_x = 8, offset_y = -8, scale = 0.5, layers = [ [0, 0, 1, 1] ] },
]
)";
	const world::ToolOverlayCatalog catalog = loadString(toml);
	const world::ToolOverlayAction* action =
		catalog.find(world::ItemKind::Egg, world::Direction::Back);
	ASSERT_NE(action, nullptr);
	EXPECT_EQ(action->action, world::ActionKind::Pickup);
	ASSERT_EQ(action->frames.size(), 4u);
	// El huevo solo se dibuja en el último cuadro (brazos arriba).
	EXPECT_TRUE(action->frames[0].layers.empty());
	EXPECT_TRUE(action->frames[2].layers.empty());
	EXPECT_EQ(action->frames[3].layers.size(), 1u);
}

TEST(ToolOverlayLoaderTest, UnknownActionIsInvalidValue) {
	const std::string toml = R"(
[egg.right]
sheet       = "Farm/egg.png"
cell        = [10, 11]
action      = "dance"
frame_ticks = 12
frames = [
  { layers = [ [0, 0, 1, 1] ] },
]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

#ifdef TALLER_TOOLS_OVERLAY_TOML
TEST(ToolOverlayLoaderTest, LoadsRealToolsOverlayFile) {
	// El archivo que usa el juego parsea y trae la pala completa: el
	// contrato de datos de "agregar herramienta = solo datos".
	world::ToolOverlayLoader loader;
	const world::ToolOverlayCatalog catalog =
		loader.loadFromFile(TALLER_TOOLS_OVERLAY_TOML);

	// Front: frame 0 = animación 1 (1x2 en [0,1]); los CUERPOS
	// RESTANTES llevan la animación 2 (1x1 en [1,2]), todos iguales.
	const world::ToolOverlayAction* front = catalog.find(
		world::ItemKind::Shovel, world::Direction::Front);
	ASSERT_NE(front, nullptr);
	EXPECT_EQ(front->frameTicks, 6);
	ASSERT_EQ(front->frames.size(), 5u);
	ASSERT_EQ(front->frames[0].layers.size(), 1u);
	EXPECT_EQ(front->frames[0].layers[0].layer.frame.col, 0);
	EXPECT_EQ(front->frames[0].layers[0].layer.frame.row, 1);
	EXPECT_EQ(front->frames[0].layers[0].layer.cols, 1);
	EXPECT_EQ(front->frames[0].layers[0].layer.rows, 2);
	for (std::size_t i = 1; i < front->frames.size(); ++i) {
		ASSERT_EQ(front->frames[i].layers.size(), 1u) << "frame " << i;
		EXPECT_EQ(front->frames[i].layers[0].layer.frame.col, 1);
		EXPECT_EQ(front->frames[i].layers[0].layer.frame.row, 2);
		EXPECT_EQ(front->frames[i].layers[0].layer.cols, 1);
		EXPECT_EQ(front->frames[i].layers[0].layer.rows, 1);
	}

	// Back: solo el primer cuerpo (brazos levantados) muestra la pala.
	const world::ToolOverlayAction* back = catalog.find(
		world::ItemKind::Shovel, world::Direction::Back);
	ASSERT_NE(back, nullptr);
	ASSERT_EQ(back->frames.size(), 5u);
	ASSERT_EQ(back->frames[0].layers.size(), 1u);
	EXPECT_EQ(back->frames[0].layers[0].layer.frame.col, 3);
	EXPECT_EQ(back->frames[0].layers[0].layer.frame.row, 1);
	EXPECT_EQ(back->frames[0].layers[0].layer.rows, 2);
	for (std::size_t i = 1; i < back->frames.size(); ++i) {
		EXPECT_TRUE(back->frames[i].layers.empty()) << "frame " << i;
	}

	// Right: el mismo sprite (1x2 en [2,1]) rotando con el movimiento.
	const world::ToolOverlayAction* right = catalog.find(
		world::ItemKind::Shovel, world::Direction::Right);
	ASSERT_NE(right, nullptr);
	ASSERT_EQ(right->frames.size(), 4u);
	for (std::size_t i = 0; i < right->frames.size(); ++i) {
		ASSERT_EQ(right->frames[i].layers.size(), 1u) << "frame " << i;
		EXPECT_EQ(right->frames[i].layers[0].layer.frame.col, 2);
		EXPECT_EQ(right->frames[i].layers[0].layer.frame.row, 1);
		EXPECT_FALSE(right->frames[i].flip);
	}
	EXPECT_LT(right->frames[0].angle, right->frames[3].angle);

	// Left: espejo exacto de right (flip, mismo sprite).
	const world::ToolOverlayAction* left = catalog.find(
		world::ItemKind::Shovel, world::Direction::Left);
	ASSERT_NE(left, nullptr);
	ASSERT_EQ(left->frames.size(), right->frames.size());
	for (std::size_t i = 0; i < left->frames.size(); ++i) {
		ASSERT_EQ(left->frames[i].layers.size(), 1u) << "frame " << i;
		EXPECT_EQ(left->frames[i].layers[0].layer.frame.col, 2);
		EXPECT_EQ(left->frames[i].layers[0].layer.frame.row, 1);
		EXPECT_TRUE(left->frames[i].flip) << "frame " << i;
	}
}
#endif

}
