#include <vector>

#include "gtest/gtest.h"
#include "sdl/render/TileLayout.h"

namespace {

using sdl::TileLayout;



TEST(TileLayoutTest, TiledFillsAreaWithCompletePieces) {
	const TileLayout layout({0, 0, 10, 10}, 4, 4);
	const std::vector<TileLayout::Placement> pieces = layout.tiled();
	ASSERT_EQ(pieces.size(), 9u);
	EXPECT_EQ(pieces.front().offsetX, 0);
	EXPECT_EQ(pieces.front().offsetY, 0);
	EXPECT_EQ(pieces.front().width, 4);
	EXPECT_EQ(pieces.front().height, 4);
}

TEST(TileLayoutTest, TiledClipsLastColumnAndRow) {
	const TileLayout layout({0, 0, 10, 10}, 4, 4);
	const std::vector<TileLayout::Placement> pieces = layout.tiled();

	const TileLayout::Placement& lastColumn = pieces[2];
	EXPECT_EQ(lastColumn.offsetX, 8);
	EXPECT_EQ(lastColumn.width, 2);

	const TileLayout::Placement& lastRow = pieces[6];
	EXPECT_EQ(lastRow.offsetY, 8);
	EXPECT_EQ(lastRow.height, 2);

	const TileLayout::Placement& corner = pieces[8];
	EXPECT_EQ(corner.width, 2);
	EXPECT_EQ(corner.height, 2);
}

TEST(TileLayoutTest, TiledOffsetsAreRelativeToTheAreaOrigin) {
	const TileLayout layout({100, 50, 10, 10}, 4, 4);
	const std::vector<TileLayout::Placement> pieces = layout.tiled();
	ASSERT_EQ(pieces.size(), 9u);
	EXPECT_EQ(pieces.front().offsetX, 0);
	EXPECT_EQ(pieces.front().offsetY, 0);
	EXPECT_EQ(pieces[1].offsetX, 4);
	EXPECT_EQ(pieces[3].offsetY, 4);
}


TEST(TileLayoutTest, StripLaysPiecesInOneRowOnly) {
	const TileLayout layout({0, 0, 10, 10}, 4, 4);
	const std::vector<TileLayout::Placement> pieces = layout.strip();

	ASSERT_EQ(pieces.size(), 3u);
	for (const TileLayout::Placement& piece : pieces) {
		EXPECT_EQ(piece.offsetY, 0);
		EXPECT_EQ(piece.height, 4);
	}
}

TEST(TileLayoutTest, StripClipsLastPieceWidth) {
	const TileLayout layout({0, 0, 10, 10}, 4, 4);
	const std::vector<TileLayout::Placement> pieces = layout.strip();
	ASSERT_EQ(pieces.size(), 3u);
	EXPECT_EQ(pieces[2].offsetX, 8);
	EXPECT_EQ(pieces[2].width, 2);
}


TEST(TileLayoutTest, PieceWithoutSideProducesNothing) {
	const TileLayout layout({0, 0, 10, 10}, 0, 4);
	EXPECT_TRUE(layout.tiled().empty());
	EXPECT_TRUE(layout.strip().empty());
}

TEST(TileLayoutTest, EmptyAreaProducesNothing) {
	const TileLayout tiled({0, 0, 0, 10}, 4, 4);
	EXPECT_TRUE(tiled.tiled().empty());

	const TileLayout strip({0, 0, 10, 0}, 4, 4);
	EXPECT_TRUE(strip.strip().empty());
}

}
