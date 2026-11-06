#include "gtest/gtest.h"

#include "game/FarmSelection.h"
#include "game/GameConstants.h"
#include "game/LocalWorldView.h"
#include "world/ContentTraits.h"

namespace {

using game::FarmSelection;
using game::LocalWorldView;
using game::ViewEntity;

constexpr int kSackSlot = 2;

world::ContentTraits sackTraits() {
	world::ContentTraits traits;
	traits.set("sack", world::ContentTrait{true});
	return traits;
}

// Jugador en (5, 5) con un saco en el slot 2 y tierra tratada al lado.
LocalWorldView viewWithSackAndDugSoil() {
	LocalWorldView view;
	view.entities.resize(game::kLocalPlayerIndex + 1);
	view.entities[game::kLocalPlayerIndex].cellX = 5;
	view.entities[game::kLocalPlayerIndex].cellY = 5;
	view.inventory.push_back(game::ViewInventorySlot{kSackSlot, "sack", 1});
	view.groundEffects.push_back(game::ViewGroundEffect{
		6, 5, world::GroundConsequence::Dug});
	return view;
}

TEST(FarmSelectionTest, HoldsTheSackOfTheActiveSlot) {
	const world::ContentTraits traits = sackTraits();
	FarmSelection farm(traits);

	farm.observe(viewWithSackAndDugSoil(), kSackSlot, world::GridCell{6, 5});

	ASSERT_TRUE(farm.carry().held().has_value());
	EXPECT_EQ(*farm.carry().held(), "sack");
}

TEST(FarmSelectionTest, MarksTheDugSoilUnderThePointerWhileHoldingASack) {
	const world::ContentTraits traits = sackTraits();
	FarmSelection farm(traits);

	farm.observe(viewWithSackAndDugSoil(), kSackSlot, world::GridCell{6, 5});

	ASSERT_TRUE(farm.spot().has_value());
	EXPECT_EQ(farm.spot()->cell.column, 6);
	EXPECT_EQ(farm.spot()->state, world::BuildPlaceState::Available);
}

TEST(FarmSelectionTest, HoldsAndMarksNothingWithAnotherSlotActive) {
	const world::ContentTraits traits = sackTraits();
	FarmSelection farm(traits);

	farm.observe(viewWithSackAndDugSoil(), 0, world::GridCell{6, 5});

	EXPECT_FALSE(farm.carry().held().has_value());
	EXPECT_FALSE(farm.spot().has_value());
}

}
