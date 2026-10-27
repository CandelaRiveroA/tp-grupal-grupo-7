#include <array>
#include <string>

#include "gtest/gtest.h"

#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"

namespace {

using world::ActionKind;
using world::CharacterCatalog;
using world::CharacterCatalogLoader;
using world::ConfigError;
using world::ConfigErrorKind;
using world::CreatureKind;
using world::Direction;

const char* const kPlayerToml = R"(
[characters.player]
sheet   = "walking.png"
grid    = [16, 32]
drawn   = [24, 48]
cadence = { step = 24, frame = 12 }

  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [[0, 0], [6, 0]] ]
    [characters.player.front.walk]
    loops  = true
    frames = [ [[1, 0], [7, 0]], [[2, 0], [8, 0]] ]

  [characters.player.back]
  mirrored = false
    [characters.player.back.idle]
    loops  = false
    frames = [ [[0, 2], [6, 2]] ]
    [characters.player.back.walk]
    loops  = true
    frames = [ [[1, 2], [7, 2]], [[2, 2], [8, 2]] ]

  [characters.player.left]
  mirrored = true
    [characters.player.left.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.left.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
    [characters.player.left.use_tool]
    loops  = false
    frames = [
      [[0, 16, 1, 2], [6, 16, 1, 2]],
      [[1, 16, 1, 2], [7, 16, 1, 2]],
      [[2, 16, 1, 2], [8, 16, 1, 2]],
      [[3, 16, 1, 2], [9, 16, 1, 2]],
      [[4, 16, 1, 2], [10, 16, 1, 2]],
    ]

  [characters.player.right]
  mirrored = false
    [characters.player.right.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.right.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
    # Picar derecha: frame = 8 -> ritmo PROPIO, distinto del default.
    [characters.player.right.use_tool]
    loops  = false
    frame  = 8
    frames = [
      [[0, 16, 1, 2], [6, 16, 1, 2]],
      [[1, 16, 1, 2], [7, 16, 1, 2]],
      [[2, 16, 1, 2], [8, 16, 1, 2]],
      [[3, 16, 1, 2], [9, 16, 1, 2]],
      [[4, 16, 1, 2], [10, 16, 1, 2]],
    ]

# Gallina mínima: el loader exige una tabla por criatura del enum.
# Walk de 1 cuadro => invariante step == frame * 1 (8 == 8 * 1).
[characters.chicken]
sheet   = "Chicken.png"
grid    = [16, 16]
drawn   = [32, 32]
cadence = { step = 8, frame = 8 }
  [characters.chicken.front]
  mirrored = false
    [characters.chicken.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.right]
  mirrored = false
    [characters.chicken.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.back]
  mirrored = false
    [characters.chicken.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.left]
  mirrored = false
    [characters.chicken.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]
)";

CharacterCatalog loadPlayer() {
	return CharacterCatalogLoader().loadFromString(kPlayerToml);
}

const world::ActionData& actionOf(const CharacterCatalog& catalog,
				  Direction facing, ActionKind action) {
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	const world::DirectionAppearance& dir =
		(*data.directions)[static_cast<int>(facing)];
	return dir.actions->at(action);
}


TEST(CharacterCatalogLoaderTest, LoadsSheetAndSizes) {
	const CharacterCatalog catalog = loadPlayer();
	EXPECT_EQ(catalog.sheetFile(CreatureKind::Player), "walking.png");
	EXPECT_EQ(catalog.sheetCellWidth(CreatureKind::Player), 16);
	EXPECT_EQ(catalog.sheetCellHeight(CreatureKind::Player), 32);
	EXPECT_EQ(catalog.drawnWidth(CreatureKind::Player), 24);
	EXPECT_EQ(catalog.drawnHeight(CreatureKind::Player), 48);
}

TEST(CharacterCatalogLoaderTest, LoadsCadence) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	EXPECT_EQ(data.cadence.stepTicks, 24);
	EXPECT_EQ(data.cadence.frameTicks, 12);
}

TEST(CharacterCatalogLoaderTest, HasPlayer) {
	const CharacterCatalog catalog = loadPlayer();
	EXPECT_TRUE(catalog.has(CreatureKind::Player));
}


TEST(CharacterCatalogLoaderTest, LoadsFrontIdleAction) {
	const CharacterCatalog catalog = loadPlayer();
	const world::ActionData& idle =
		actionOf(catalog, Direction::Front, ActionKind::Idle);
	ASSERT_EQ(idle.frames.size(), 1u);
	EXPECT_FALSE(idle.loops);
	ASSERT_EQ(idle.frames[0].size(), 2u);
	EXPECT_EQ(idle.frames[0][0].frame.col, 0);
	EXPECT_EQ(idle.frames[0][0].frame.row, 0);
	EXPECT_EQ(idle.frames[0][1].frame.col, 6);
}

TEST(CharacterCatalogLoaderTest, LoadsFrontWalkAction) {
	const CharacterCatalog catalog = loadPlayer();
	const world::ActionData& walk =
		actionOf(catalog, Direction::Front, ActionKind::Walk);
	ASSERT_EQ(walk.frames.size(), 2u);
	EXPECT_TRUE(walk.loops);
	EXPECT_EQ(walk.frames[0][0].frame.col, 1);
	EXPECT_EQ(walk.frames[0][1].frame.col, 7);
	EXPECT_EQ(walk.frames[1][0].frame.col, 2);
	EXPECT_EQ(walk.frames[1][1].frame.col, 8);
}


TEST(CharacterCatalogLoaderTest, TwoIntLayerDefaultsToOneCell) {
	const CharacterCatalog catalog = loadPlayer();
	const world::ActionData& idle =
		actionOf(catalog, Direction::Front, ActionKind::Idle);
	ASSERT_EQ(idle.frames[0].size(), 2u);
	EXPECT_EQ(idle.frames[0][0].cols, 1);
	EXPECT_EQ(idle.frames[0][0].rows, 1);
}

TEST(CharacterCatalogLoaderTest, FourIntLayerCarriesItsCellSize) {
	const CharacterCatalog catalog = loadPlayer();
	const world::ActionData& useTool =
		actionOf(catalog, Direction::Right, ActionKind::UseTool);
	EXPECT_FALSE(useTool.loops);
	EXPECT_EQ(useTool.frames.size(), 5u);
	EXPECT_EQ(useTool.frames[0][0].frame.col, 0);
	EXPECT_EQ(useTool.frames[0][0].frame.row, 16);
	EXPECT_EQ(useTool.frames[0][0].cols, 1);
	EXPECT_EQ(useTool.frames[0][0].rows, 2);
	EXPECT_EQ(useTool.frames[0][1].frame.col, 6);
	EXPECT_EQ(useTool.frames[0][1].cols, 1);
	EXPECT_EQ(useTool.frames[0][1].rows, 2);
	EXPECT_EQ(useTool.frames[4][0].frame.col, 4);
	EXPECT_EQ(useTool.frames[4][1].frame.col, 10);
}

TEST(CharacterCatalogLoaderTest, MirroredDirectionAlsoLoadsUseTool) {
	const CharacterCatalog catalog = loadPlayer();
	const world::ActionData& useTool =
		actionOf(catalog, Direction::Left, ActionKind::UseTool);
	EXPECT_EQ(useTool.frames.size(), 5u);
	EXPECT_FALSE(useTool.loops);
}


TEST(CharacterCatalogLoaderTest, ActionWithOwnFrameOverridesTheCadence) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	EXPECT_EQ(data.cadence.frameTicks, 12);
	EXPECT_EQ(actionOf(catalog, Direction::Right, ActionKind::UseTool)
			  .frameTicks,
		  8);
}

TEST(CharacterCatalogLoaderTest, ActionWithoutFrameInheritsTheCadence) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	EXPECT_EQ(data.cadence.frameTicks, 12);
	EXPECT_EQ(actionOf(catalog, Direction::Front, ActionKind::Walk)
			  .frameTicks,
		  12);
	EXPECT_EQ(actionOf(catalog, Direction::Left, ActionKind::UseTool)
			  .frameTicks,
		  12);
}

TEST(CharacterCatalogLoaderTest, ExplicitWalkFrameIsReadAndValidated) {
	std::string toml = kPlayerToml;
	const std::string from = "[characters.player.front.walk]\n    loops  = true";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(),
		     from + "\n    frame  = 12");
	const CharacterCatalog catalog =
		CharacterCatalogLoader().loadFromString(toml);
	EXPECT_EQ(actionOf(catalog, Direction::Front, ActionKind::Walk)
			  .frameTicks,
		  12);
}

TEST(CharacterCatalogLoaderTest, LeftIsMirroredRightIsNot) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	const world::DirectionAppearance& left =
		(*data.directions)[static_cast<int>(Direction::Left)];
	const world::DirectionAppearance& right =
		(*data.directions)[static_cast<int>(Direction::Right)];
	EXPECT_TRUE(left.mirrored);
	EXPECT_FALSE(right.mirrored);
}


TEST(CharacterCatalogLoaderTest, LoadedDataDrivesAppearance) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);

	const world::EntityAppearance idle = world::appearanceFor(
		data, Direction::Front, ActionKind::Idle, 0);
	EXPECT_EQ((*idle.pose)[0].frame.col, 0);

	const world::EntityAppearance walking = world::appearanceFor(
		data, Direction::Front, ActionKind::Walk, 0);
	EXPECT_EQ((*walking.pose)[0].frame.col, 1);
}

TEST(CharacterCatalogLoaderTest, WalkCycleFitsExactlyOneStep) {
	const CharacterCatalog catalog = loadPlayer();
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	EXPECT_GT(data.cadence.frameTicks, 0);
	for (const world::DirectionAppearance& dir : *data.directions) {
		const world::ActionData& walk = dir.actions->at(ActionKind::Walk);
		EXPECT_EQ(data.cadence.stepTicks,
			  data.cadence.frameTicks *
				  static_cast<int>(walk.frames.size()));
	}
}


ConfigErrorKind kindOfError(const std::string& toml) {
	try {
		CharacterCatalogLoader().loadFromString(toml);
	} catch (const ConfigError& error) {
		return error.kind();
	}
	ADD_FAILURE() << "se esperaba ConfigError";
	return ConfigErrorKind::UnreadableInput;
}

TEST(CharacterCatalogLoaderTest, MissingCreatureThrowsMissingStructure) {
	const std::string toml = "[characters.other]\nsheet = \"x.png\"\n";
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::MissingStructure);
}

TEST(CharacterCatalogLoaderTest, MissingSheetThrowsMissingStructure) {
	const std::string toml = R"(
[characters.player]
grid    = [16, 32]
drawn   = [24, 48]
cadence = { step = 24, frame = 12 }
)";
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::MissingStructure);
}

TEST(CharacterCatalogLoaderTest, MissingIdleThrowsMissingStructure) {
	std::string toml = kPlayerToml;
	const std::string from = "[characters.player.front.idle]";
	const std::string to = "[characters.player.front.idle_vieja]";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(), to);
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::MissingStructure);
}

TEST(CharacterCatalogLoaderTest, EmptySheetThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "sheet   = \"walking.png\"";
	toml.replace(toml.find(from), from.size(), "sheet   = \"\"");
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, FrameZeroThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "cadence = { step = 24, frame = 12 }";
	toml.replace(toml.find(from), from.size(),
		     "cadence = { step = 24, frame = 0 }");
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, StepNotMultipleOfFrameThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "cadence = { step = 24, frame = 12 }";
	toml.replace(toml.find(from), from.size(),
		     "cadence = { step = 25, frame = 12 }");
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, StepFrameCountMismatchThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "cadence = { step = 24, frame = 12 }";
	toml.replace(toml.find(from), from.size(),
		     "cadence = { step = 12, frame = 12 }");
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, InvalidTomlThrowsUnreadableInput) {
	EXPECT_EQ(kindOfError("not = = toml"),
		  ConfigErrorKind::UnreadableInput);
}

TEST(CharacterCatalogLoaderTest, ThreeIntLayerThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "frames = [ [[0, 1], [6, 1]] ]";
	const std::string to = "frames = [ [[0, 1, 1], [6, 1]] ]";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(), to);
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, ZeroLayerSizeThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "frames = [ [[0, 1], [6, 1]] ]";
	const std::string to = "frames = [ [[0, 1, 1, 0], [6, 1]] ]";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(), to);
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, ZeroActionFrameThrowsInvalidValue) {
	std::string toml = kPlayerToml;
	const std::string from = "[characters.player.front.idle]";
	const std::string to = from + "\n    frame  = 0";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(), to);
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}

TEST(CharacterCatalogLoaderTest, WalkFrameBreakingTheStepInvariantThrows) {
	std::string toml = kPlayerToml;
	const std::string from =
		"[characters.player.front.walk]\n    loops  = true";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(),
		     from + "\n    frame  = 8");
	EXPECT_EQ(kindOfError(toml), ConfigErrorKind::InvalidValue);
}


#ifdef TALLER_CHARACTERS_TOML
TEST(CharacterCatalogLoaderTest, LoadsRealCharactersFile) {
	CharacterCatalogLoader loader;
	const CharacterCatalog catalog =
		loader.loadFromFile(TALLER_CHARACTERS_TOML);
	ASSERT_TRUE(catalog.has(CreatureKind::Player));
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);
	EXPECT_EQ(data.cadence.stepTicks, 24);
	EXPECT_EQ(data.cadence.frameTicks, 12);
	EXPECT_EQ(catalog.sheetFile(CreatureKind::Player),
		  "Players/main/Character_Base.png");
	EXPECT_EQ(catalog.drawnWidth(CreatureKind::Player), 32);
	EXPECT_EQ(catalog.drawnHeight(CreatureKind::Player), 64);
	const world::ActionData& useTool =
		actionOf(catalog, Direction::Right, ActionKind::UseTool);
	EXPECT_FALSE(useTool.loops);
	EXPECT_EQ(useTool.frames.size(), 5u);
	EXPECT_EQ(useTool.frames[0][0].rows, 2);
}

TEST(CharacterCatalogLoaderTest, RealFileToolUseDurationIsUniform) {
	CharacterCatalogLoader loader;
	const CharacterCatalog catalog =
		loader.loadFromFile(TALLER_CHARACTERS_TOML);
	const world::CreatureData& data = catalog.dataFor(CreatureKind::Player);

	int expectedTicks = -1;
	for (const world::DirectionAppearance& dir : *data.directions) {
		const auto it = dir.actions->find(ActionKind::UseTool);
		if (it == dir.actions->end()) {
			continue;
		}
		const int ticks = static_cast<int>(it->second.frames.size()) *
				  it->second.frameTicks;
		if (expectedTicks < 0) {
			expectedTicks = ticks;
			continue;
		}
		EXPECT_EQ(ticks, expectedTicks)
			<< "use_tool de una dirección dura distinto que "
			<< "la primera que la define";
	}
	ASSERT_GT(expectedTicks, 0);
}

TEST(CharacterCatalogLoaderTest, RealFileLoadsChicken) {
	CharacterCatalogLoader loader;
	const CharacterCatalog catalog =
		loader.loadFromFile(TALLER_CHARACTERS_TOML);
	ASSERT_TRUE(catalog.has(CreatureKind::Chicken));

	EXPECT_EQ(catalog.sheetFile(CreatureKind::Chicken),
		  "Farm/Chicken.png");
	EXPECT_EQ(catalog.sheetCellWidth(CreatureKind::Chicken), 16);
	EXPECT_EQ(catalog.sheetCellHeight(CreatureKind::Chicken), 16);
	EXPECT_EQ(catalog.drawnWidth(CreatureKind::Chicken), 32);
	EXPECT_EQ(catalog.drawnHeight(CreatureKind::Chicken), 32);

	const world::CreatureData& data =
		catalog.dataFor(CreatureKind::Chicken);
	EXPECT_EQ(data.cadence.stepTicks, 32);
	EXPECT_EQ(data.cadence.frameTicks, 8);

	// Caminata relevada: 4 cuadros por dirección. Filas de la hoja en
	// orden de hoja front/right/back/left; el enum es Front, Back,
	// Left, Right (0: front -> fila 0, 1: back -> fila 2, 2: left ->
	// fila 3, 3: right -> fila 1).
	const std::array<int, 4> kWalkRows = {0, 2, 3, 1};
	for (std::size_t d = 0; d < kWalkRows.size(); ++d) {
		const world::DirectionAppearance& dir =
			(*data.directions)[d];
		EXPECT_FALSE(dir.mirrored)
			<< "la hoja trae las cuatro direcciones explícitas";
		const auto walk = dir.actions->find(ActionKind::Walk);
		ASSERT_NE(walk, dir.actions->end());
		EXPECT_EQ(walk->second.frames.size(), 4u);
		for (std::size_t f = 0; f < walk->second.frames.size(); ++f) {
			EXPECT_EQ(walk->second.frames[f][0].frame.col,
				  static_cast<int>(f));
			EXPECT_EQ(walk->second.frames[f][0].frame.row,
				  kWalkRows[d]);
		}
	}

	// Idle = pose "parada" de cada dirección (capa única 1x1), en
	// orden del enum (Front, Back, Left, Right): (0,4), (0,5), (2,5),
	// (2,4) según el relevamiento.
	const std::array<int, 4> kIdleRows = {4, 5, 5, 4};
	const std::array<int, 4> kIdleCols = {0, 0, 2, 2};
	for (std::size_t d = 0; d < kIdleCols.size(); ++d) {
		const world::DirectionAppearance& dir =
			(*data.directions)[d];
		const auto idle = dir.actions->find(ActionKind::Idle);
		ASSERT_NE(idle, dir.actions->end());
		EXPECT_EQ(idle->second.frames.size(), 1u);
		EXPECT_EQ(idle->second.frames[0][0].frame.col, kIdleCols[d]);
		EXPECT_EQ(idle->second.frames[0][0].frame.row, kIdleRows[d]);
	}

	// La gallina no declara use_tool: pedirlo cae a idle (sin romper).
	const world::EntityAppearance idleFallBack =
		world::appearanceFor(data, Direction::Front,
				     ActionKind::UseTool, 0);
	EXPECT_EQ((*idleFallBack.pose)[0].frame.col, 0);
	EXPECT_EQ((*idleFallBack.pose)[0].frame.row, 4);
}
#endif

}
