#include "gtest/gtest.h"
#include "world/ScenePixelMapper.h"
#include "world/TileLayerLayout.h"

namespace {

constexpr int kCell = 32;

class TileLayerLayoutTest : public ::testing::Test {
protected:
	TileLayerLayoutTest() : layout_(world::ScenePixelMapper(kCell)) {}

	world::TileLayerLayout layout_;
	const world::GridSpan size_{10, 8};
};

TEST_F(TileLayerLayoutTest, WholeLayerVisibleCoversEveryCell) {
	const auto range =
		layout_.visible(size_, world::PixelRect{0, 0, 10 * kCell, 8 * kCell});
	EXPECT_EQ(range.firstColumn, 0);
	EXPECT_EQ(range.lastColumn, 9);
	EXPECT_EQ(range.firstRow, 0);
	EXPECT_EQ(range.lastRow, 7);
}

TEST_F(TileLayerLayoutTest, OnlyTheCellsTouchedByTheViewportAreReturned) {
	// Viewport desplazado a mitad de la celda (2,1): toca de la celda 2 a
	// la 5 en columnas (4 celdas de ancho + el borde) y 1..3 en filas.
	const auto range = layout_.visible(
		size_, world::PixelRect{2 * kCell + 16, kCell + 1, 3 * kCell, 2 * kCell});
	EXPECT_EQ(range.firstColumn, 2);
	EXPECT_EQ(range.lastColumn, 5);
	EXPECT_EQ(range.firstRow, 1);
	EXPECT_EQ(range.lastRow, 3);
}

TEST_F(TileLayerLayoutTest, ViewportExactlyOnCellBordersDoesNotTouchTheNextCell) {
	const auto range = layout_.visible(
		size_, world::PixelRect{kCell, kCell, 2 * kCell, kCell});
	EXPECT_EQ(range.firstColumn, 1);
	EXPECT_EQ(range.lastColumn, 2);
	EXPECT_EQ(range.firstRow, 1);
	EXPECT_EQ(range.lastRow, 1);
}

TEST_F(TileLayerLayoutTest, ViewportLargerThanTheLayerIsClamped) {
	const auto range = layout_.visible(
		size_, world::PixelRect{-100, -100, 5000, 5000});
	EXPECT_EQ(range.firstColumn, 0);
	EXPECT_EQ(range.lastColumn, 9);
	EXPECT_EQ(range.firstRow, 0);
	EXPECT_EQ(range.lastRow, 7);
}

TEST_F(TileLayerLayoutTest, ViewportOutsideTheLayerIsEmpty) {
	EXPECT_TRUE(layout_
			    .visible(size_, world::PixelRect{20 * kCell, 0, kCell, kCell})
			    .empty());
	EXPECT_TRUE(layout_
			    .visible(size_, world::PixelRect{-5 * kCell, 0, kCell, kCell})
			    .empty());
}

TEST_F(TileLayerLayoutTest, DegenerateInputsAreEmpty) {
	EXPECT_TRUE(layout_.visible({0, 0}, world::PixelRect{0, 0, 100, 100}).empty());
	EXPECT_TRUE(layout_.visible(size_, world::PixelRect{0, 0, 0, 100}).empty());
}

TEST_F(TileLayerLayoutTest, CellRectIsTheCellInWorldPixels) {
	const world::PixelRect rect = layout_.cellRect(3, 2);
	EXPECT_EQ(rect.x, 3 * kCell);
	EXPECT_EQ(rect.y, 2 * kCell);
	EXPECT_EQ(rect.width, kCell);
	EXPECT_EQ(rect.height, kCell);
}

}  // namespace
