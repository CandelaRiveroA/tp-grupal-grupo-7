#include "gtest/gtest.h"

#include <optional>

#include "game/SoundCue.h"
#include "world/CreatureCatalog.h"

namespace {

TEST(SoundCueCatalogTest, NameOfCueIsItsTomlName) {
	// La lista `sounds` de scenes.toml declara los cues por este nombre.
	const game::SoundCueCatalog catalog;
	EXPECT_STREQ(catalog.nameOf(game::CueKind::ChickenCluck),
		     "chicken_cluck");
}

TEST(SoundCueCatalogTest, ChickenEmitsItsCluck) {
	const game::SoundCueCatalog catalog;
	const std::optional<game::CueKind> cue =
		catalog.cueOf(world::CreatureKind::Chicken);
	ASSERT_TRUE(cue.has_value());
	EXPECT_EQ(*cue, game::CueKind::ChickenCluck);
}

TEST(SoundCueCatalogTest, SilentCreatureHasNoCue) {
	// El jugador no cacarea: sin fila, nullopt (regla silenciosa).
	const game::SoundCueCatalog catalog;
	EXPECT_FALSE(
		catalog.cueOf(world::CreatureKind::Player).has_value());
}

}
