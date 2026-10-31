#include "gtest/gtest.h"

#include "world/ElementLayout.h"

namespace {

using world::DrawMode;
using world::DrawSpec;
using world::ElementLayout;
using world::ElementPlacement;
using world::GridCell;
using world::GridSpan;
using world::PixelRect;
using world::SpriteRegion;
using world::TileSetDef;

ElementPlacement makeElement(DrawMode mode, int scaleX, int scaleY) {
	return ElementPlacement{
		"tiles",
		SpriteRegion{GridCell{0, 0}, GridSpan{2, 2}},
		DrawSpec{mode, scaleX, scaleY},
		GridCell{1, 2},
		GridSpan{3, 4},
	};
}

TEST(ElementLayoutTest, ComputesPieceSizeWithRegionAndScale) {
	const TileSetDef tileset{"sheet.png", 16, 24};
	const ElementPlacement element = makeElement(DrawMode::Single, 2, 3);
	const ElementLayout layout(tileset, element, PixelRect{10, 20, 30, 40});

	EXPECT_EQ(layout.pieceWidth(), 16 * 2 * 2);
	EXPECT_EQ(layout.pieceHeight(), 24 * 2 * 3);
}

TEST(ElementLayoutTest, SingleBoundsUseThePieceSize) {
	const TileSetDef tileset{"sheet.png", 16, 24};
	const ElementPlacement element = makeElement(DrawMode::Single, 1, 1);
	const ElementLayout layout(tileset, element, PixelRect{10, 20, 30, 40});

	const PixelRect bounds = layout.bounds();
	EXPECT_EQ(bounds.x, 10);
	EXPECT_EQ(bounds.y, 20);
	EXPECT_EQ(bounds.width, 32);
	EXPECT_EQ(bounds.height, 48);
}

TEST(ElementLayoutTest, TileBoundsUseTheArea) {
	const TileSetDef tileset{"sheet.png", 16, 24};
	const ElementPlacement element = makeElement(DrawMode::Tile, 1, 1);
	const ElementLayout layout(tileset, element, PixelRect{10, 20, 30, 40});

	const PixelRect bounds = layout.bounds();
	EXPECT_EQ(bounds.x, 10);
	EXPECT_EQ(bounds.y, 20);
	EXPECT_EQ(bounds.width, 30);
	EXPECT_EQ(bounds.height, 40);
}

TEST(ElementLayoutTest, StripBoundsUseTheAreaWidthAndPieceHeight) {
	const TileSetDef tileset{"sheet.png", 16, 24};
	const ElementPlacement element =
		makeElement(DrawMode::HorizontalStrip, 1, 1);
	const ElementLayout layout(tileset, element, PixelRect{10, 20, 30, 40});

	const PixelRect bounds = layout.bounds();
	EXPECT_EQ(bounds.x, 10);
	EXPECT_EQ(bounds.y, 20);
	EXPECT_EQ(bounds.width, 30);
	EXPECT_EQ(bounds.height, 48);
}

}  // namespace
