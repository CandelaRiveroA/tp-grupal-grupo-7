#include <string>

#include "gtest/gtest.h"

#include "world/ConfigError.h"
#include "world/DialogCatalog.h"
#include "world/DialogConfigLoader.h"

namespace {

using world::ConfigError;
using world::ConfigErrorKind;
using world::DialogCatalog;
using world::DialogConfigLoader;
using world::Indication;

const char* const kDialogsToml = R"(
[ready_to_milk]
anchor_offset = [0, -64]

  [ready_to_milk.animation]
  sheet       = "ui/emotes.png"
  cell        = [16, 16]
  drawn       = [64, 64]
  frame_ticks = 6
  loops       = false
  frames      = [ [0, 0], [1, 0], [2, 0], [3, 0] ]

  [ready_to_milk.content]
  sheet = "Farm/large_milk.png"
  cell  = [53, 64]
  at    = [0, 0]
)";

TEST(DialogConfigLoaderTest, LoadsDialogWithAnimationAndContent) {
	const DialogCatalog catalog =
			DialogConfigLoader().loadFromString(kDialogsToml);

	const world::DialogData* dialog =
			catalog.find(Indication::ReadyToMilk);
	ASSERT_NE(dialog, nullptr);

	// Animación: hoja común + los 4 frames y su ritmo.
	const world::DialogAnimation& animation = dialog->animation;
	EXPECT_EQ(animation.sprite.sheetImage, "ui/emotes.png");
	EXPECT_EQ(animation.sprite.displayWidth(), 64);
	EXPECT_EQ(animation.sprite.displayHeight(), 64);
	ASSERT_EQ(animation.frames.size(), 4u);
	EXPECT_EQ(animation.frames[0].col, 0);
	EXPECT_EQ(animation.frames[3].col, 3);
	EXPECT_EQ(animation.frameTicks, 6);
	EXPECT_FALSE(animation.loops);

	// Contenido centrado adentro (la leche).
	EXPECT_EQ(dialog->content.sheetImage, "Farm/large_milk.png");
	EXPECT_EQ(dialog->content.displayWidth(), 53);
	EXPECT_EQ(dialog->content.displayHeight(), 64);

	// Ancla sobre la entidad.
	EXPECT_EQ(dialog->anchorOffset.x, 0);
	EXPECT_EQ(dialog->anchorOffset.y, -64);
}

TEST(DialogConfigLoaderTest, AnimationIsOneShotByDefault) {
	const DialogCatalog catalog =
			DialogConfigLoader().loadFromString(kDialogsToml);
	const world::DialogData* dialog =
			catalog.find(Indication::ReadyToMilk);
	ASSERT_NE(dialog, nullptr);

	// Avanza hasta el último frame y se queda ahí ("informa").
	EXPECT_EQ(dialog->animation.frameIndexAt(0), 0);
	EXPECT_EQ(dialog->animation.frameIndexAt(6), 1);
	EXPECT_EQ(dialog->animation.frameIndexAt(18), 3);
	EXPECT_EQ(dialog->animation.frameIndexAt(500), 3);
}

TEST(DialogConfigLoaderTest, ZeroFrameTicksIsInvalidValue) {
	const std::string toml = R"(
[ready_to_milk]
anchor_offset = [0, -64]

  [ready_to_milk.animation]
  sheet       = "ui/emotes.png"
  cell        = [16, 16]
  frame_ticks = 0
  frames      = [ [0, 0] ]

  [ready_to_milk.content]
  sheet = "Farm/large_milk.png"
  cell  = [53, 64]
  at    = [0, 0]
)";
	try {
		DialogConfigLoader().loadFromString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(error.kind(), ConfigErrorKind::InvalidValue);
	}
}

TEST(DialogConfigLoaderTest, EmptyFramesIsInvalidValue) {
	const std::string toml = R"(
[ready_to_milk]
anchor_offset = [0, -64]

  [ready_to_milk.animation]
  sheet       = "ui/emotes.png"
  cell        = [16, 16]
  frame_ticks = 6
  frames      = []

  [ready_to_milk.content]
  sheet = "Farm/large_milk.png"
  cell  = [53, 64]
  at    = [0, 0]
)";
	try {
		DialogConfigLoader().loadFromString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(error.kind(), ConfigErrorKind::InvalidValue);
	}
}

}  // namespace
