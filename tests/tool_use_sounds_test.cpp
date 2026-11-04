#include "gtest/gtest.h"

#include "core/ToolUseSounds.h"

namespace {

using core::ToolUseSounds;
using world::ItemKind;

TEST(ToolUseSoundsTest, PlaysTheToolSoundOnceWhenTheUseStarts) {
	ToolUseSounds sounds;

	const auto first = sounds.update(ItemKind::WateringCan);
	ASSERT_TRUE(first.has_value());
	EXPECT_EQ(*first, sdl::SoundId::WateringCanPour);

	// Mientras dura el uso no se repite.
	EXPECT_FALSE(sounds.update(ItemKind::WateringCan).has_value());
	EXPECT_FALSE(sounds.update(ItemKind::WateringCan).has_value());
}

TEST(ToolUseSoundsTest, PlaysAgainOnTheNextUse) {
	ToolUseSounds sounds;

	sounds.update(ItemKind::Shovel);
	sounds.update(std::nullopt);

	const auto again = sounds.update(ItemKind::Shovel);
	ASSERT_TRUE(again.has_value());
	EXPECT_EQ(*again, sdl::SoundId::ShovelDig);
}

TEST(ToolUseSoundsTest, ToolWithoutRowIsSilent) {
	ToolUseSounds sounds;

	EXPECT_FALSE(sounds.update(ItemKind::Pickaxe).has_value());
}

TEST(ToolUseSoundsTest, NoToolInUseIsSilent) {
	ToolUseSounds sounds;

	EXPECT_FALSE(sounds.update(std::nullopt).has_value());
}

}
