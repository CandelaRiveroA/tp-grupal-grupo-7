#include "game/LocalWorldView.h"
#include "game/NpcPlacementResolver.h"
#include "gtest/gtest.h"
#include "world/SceneDescription.h"

namespace {

TEST(NpcPlacementResolverTest, WithoutAnythingThereAreNoCells) {
	const world::NpcCells cells = game::NpcPlacementResolver().cellsFrom(
		game::LocalWorldView{}, world::SceneDescription{});
	EXPECT_TRUE(cells.empty());
}

TEST(NpcPlacementResolverTest, TheSceneGivesTheDefaultCells) {
	world::SceneDescription scene;
	scene.npcCells["pierre"] = {2, 3};
	const world::NpcCells cells = game::NpcPlacementResolver().cellsFrom(
		game::LocalWorldView{}, scene);
	ASSERT_EQ(cells.count("pierre"), 1u);
	EXPECT_EQ(cells.at("pierre").column, 2);
	EXPECT_EQ(cells.at("pierre").row, 3);
}

TEST(NpcPlacementResolverTest, TheStateSourceOverridesTheScene) {
	world::SceneDescription scene;
	scene.npcCells["pierre"] = {2, 3};
	scene.npcCells["marnie"] = {5, 5};
	game::LocalWorldView view;
	view.npcPlacements.push_back(game::ViewNpcPlacement{"pierre", 9, 8});
	const world::NpcCells cells =
		game::NpcPlacementResolver().cellsFrom(view, scene);
	EXPECT_EQ(cells.at("pierre").column, 9);
	EXPECT_EQ(cells.at("pierre").row, 8);
	// El que la fuente no menciona conserva la celda del mapa.
	EXPECT_EQ(cells.at("marnie").column, 5);
}

}  // namespace
