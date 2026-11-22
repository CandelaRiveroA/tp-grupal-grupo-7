#include "gtest/gtest.h"
#include "world/FurnitureLayout.h"

namespace {

constexpr int kCell = 32;

world::SpriteVisual visualOf(int cols, int rows) {
	world::SpriteVisual visual;
	visual.cols = cols;
	visual.rows = rows;
	return visual;
}

TEST(FurnitureLayoutTest, OneCellFurnitureFillsItsCell) {
	const world::PixelRect box =
		world::FurnitureLayout(world::ScenePixelMapper(kCell))
			.box(visualOf(1, 1), {3, 4});
	EXPECT_EQ(box.x, 3 * kCell);
	EXPECT_EQ(box.y, 4 * kCell);
	EXPECT_EQ(box.width, kCell);
	EXPECT_EQ(box.height, kCell);
}

TEST(FurnitureLayoutTest, TallFurnitureGrowsUpFromItsBaseCell) {
	// Planta 1x2 con la base en (5,6): ocupa las filas 5 y 6.
	const world::PixelRect box =
		world::FurnitureLayout(world::ScenePixelMapper(kCell))
			.box(visualOf(1, 2), {5, 6});
	EXPECT_EQ(box.x, 5 * kCell);
	EXPECT_EQ(box.y, 5 * kCell);
	EXPECT_EQ(box.width, kCell);
	EXPECT_EQ(box.height, 2 * kCell);
}

TEST(FurnitureLayoutTest, WideFurnitureGrowsRightFromItsBaseCell) {
	// Biblioteca 2x3 con la base en (2,8): columnas 2-3, filas 6-8.
	const world::PixelRect box =
		world::FurnitureLayout(world::ScenePixelMapper(kCell))
			.box(visualOf(2, 3), {2, 8});
	EXPECT_EQ(box.x, 2 * kCell);
	EXPECT_EQ(box.y, 6 * kCell);
	EXPECT_EQ(box.width, 2 * kCell);
	EXPECT_EQ(box.height, 3 * kCell);
}

TEST(FurnitureLayoutTest, BoxFollowsTheCellPixelSize) {
	const world::PixelRect box =
		world::FurnitureLayout(world::ScenePixelMapper(48))
			.box(visualOf(1, 2), {1, 3});
	EXPECT_EQ(box.width, 48);
	EXPECT_EQ(box.height, 96);
	EXPECT_EQ(box.y, 2 * 48);
}

}  // namespace
