#include "gtest/gtest.h"
#include "sdl/render/TileLayout.h"

namespace {

using sdl::TileLayout;
using world::PixelRect;

TEST(TileLayoutTest, VisibleTiledReturnsEveryPieceForAFullClip) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 4, 4);
	const TileLayout::Range range =
		layout.visibleTiled(PixelRect{0, 0, 10, 10});

	EXPECT_EQ(range.firstColumn, 0);
	EXPECT_EQ(range.lastColumn, 2);
	EXPECT_EQ(range.firstRow, 0);
	EXPECT_EQ(range.lastRow, 2);
	EXPECT_FALSE(range.empty());
}

TEST(TileLayoutTest, TiledAtClipsLastColumnAndRow) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 4, 4);

	const TileLayout::Placement lastColumn = layout.tiledAt(2, 0);
	EXPECT_EQ(lastColumn.offsetX, 8);
	EXPECT_EQ(lastColumn.width, 2);

	const TileLayout::Placement lastRow = layout.tiledAt(0, 2);
	EXPECT_EQ(lastRow.offsetY, 8);
	EXPECT_EQ(lastRow.height, 2);

	const TileLayout::Placement corner = layout.tiledAt(2, 2);
	EXPECT_EQ(corner.width, 2);
	EXPECT_EQ(corner.height, 2);
}

TEST(TileLayoutTest, TiledOffsetsAreRelativeToTheAreaOrigin) {
	const TileLayout layout(PixelRect{100, 50, 10, 10}, 4, 4);

	EXPECT_EQ(layout.tiledAt(0, 0).offsetX, 0);
	EXPECT_EQ(layout.tiledAt(0, 0).offsetY, 0);
	EXPECT_EQ(layout.tiledAt(1, 0).offsetX, 4);
	EXPECT_EQ(layout.tiledAt(0, 1).offsetY, 4);
}

TEST(TileLayoutTest, VisibleTiledSkipsPiecesOutsideTheClip) {
	const TileLayout layout(PixelRect{0, 0, 40, 40}, 10, 10);
	const TileLayout::Range range =
		layout.visibleTiled(PixelRect{15, 15, 10, 10});

	EXPECT_EQ(range.firstColumn, 1);
	EXPECT_EQ(range.lastColumn, 2);
	EXPECT_EQ(range.firstRow, 1);
	EXPECT_EQ(range.lastRow, 2);
}

TEST(TileLayoutTest, VisibleTiledIsEmptyWhenTheClipDoesNotIntersect) {
	const TileLayout layout(PixelRect{0, 0, 40, 40}, 10, 10);
	const TileLayout::Range range =
		layout.visibleTiled(PixelRect{100, 100, 10, 10});

	EXPECT_TRUE(range.empty());
}

TEST(TileLayoutTest, StripLaysPiecesInOneRowOnly) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 4, 4);
	const TileLayout::Range range =
		layout.visibleStrip(PixelRect{0, 0, 10, 10});

	EXPECT_EQ(range.firstRow, 0);
	EXPECT_EQ(range.lastRow, 0);
	for (int column = range.firstColumn; column <= range.lastColumn;
	     ++column) {
		const TileLayout::Placement piece = layout.stripAt(column);
		EXPECT_EQ(piece.offsetY, 0);
		EXPECT_EQ(piece.height, 4);
	}
}

TEST(TileLayoutTest, StripClipsLastPieceWidth) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 4, 4);
	const TileLayout::Placement piece = layout.stripAt(2);

	EXPECT_EQ(piece.offsetX, 8);
	EXPECT_EQ(piece.width, 2);
}

TEST(TileLayoutTest, StripIsEmptyWhenTheClipDoesNotIntersectItsRow) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 4, 4);
	EXPECT_TRUE(layout.visibleStrip(PixelRect{0, 100, 10, 10}).empty());
}

TEST(TileLayoutTest, PieceWithoutSideProducesNothing) {
	const TileLayout layout(PixelRect{0, 0, 10, 10}, 0, 4);
	EXPECT_TRUE(layout.visibleTiled(PixelRect{0, 0, 10, 10}).empty());
	EXPECT_TRUE(layout.visibleStrip(PixelRect{0, 0, 10, 10}).empty());
}

TEST(TileLayoutTest, EmptyAreaProducesNothing) {
	const TileLayout tiled(PixelRect{0, 0, 0, 10}, 4, 4);
	EXPECT_TRUE(tiled.visibleTiled(PixelRect{0, 0, 10, 10}).empty());

	const TileLayout strip(PixelRect{0, 0, 10, 0}, 4, 4);
	EXPECT_TRUE(strip.visibleStrip(PixelRect{0, 0, 10, 10}).empty());
}

}  // namespace
