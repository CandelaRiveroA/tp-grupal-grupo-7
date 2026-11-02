#include "gtest/gtest.h"

#include "world/CharacterLayout.h"

namespace {

using world::CharacterLayout;
using world::SheetCell;
using world::Layer;
using world::LayerFrame;
using world::PixelRect;

LayerFrame makeLayer(int cols, int rows) {
	return LayerFrame{Layer::Body, SheetCell{0, 0}, cols, rows};
}

TEST(CharacterLayoutTest, BodyIsCenteredHorizontallyInTheBox) {
	const CharacterLayout layout(PixelRect{10, 20, 200, 100}, 64, 64);

	EXPECT_EQ(layout.bodyX(), 10 + (200 - 64) / 2);
}

TEST(CharacterLayoutTest, BodyTopAndBottomUseDrawnHeight) {
	const CharacterLayout layout(PixelRect{10, 20, 200, 100}, 64, 48);

	EXPECT_EQ(layout.bodyBottom(), 120);
	EXPECT_EQ(layout.bodyTop(), 120 - 48);
}

TEST(CharacterLayoutTest, TwoRowLayerAnchorsItsBottomAtBodyBottom) {
	const CharacterLayout layout(PixelRect{0, 0, 128, 128}, 64, 64);

	const PixelRect rect = layout.layerRect(makeLayer(1, 2), makeLayer(1, 2));
	EXPECT_EQ(rect.x, layout.bodyX());
	EXPECT_EQ(rect.height, 64);
	EXPECT_EQ(rect.y + rect.height, layout.bodyBottom());
}

TEST(CharacterLayoutTest, OneByOneLayerSitsOnBodyBottom) {
	const CharacterLayout layout(PixelRect{0, 0, 128, 128}, 64, 64);

	const PixelRect rect = layout.layerRect(makeLayer(1, 1), makeLayer(1, 2));
	EXPECT_EQ(rect.x, layout.bodyX());
	EXPECT_EQ(rect.height, 32);
	EXPECT_EQ(rect.y, layout.bodyBottom() - 32);
	EXPECT_EQ(rect.y + rect.height, layout.bodyBottom());
}

TEST(CharacterLayoutTest, InvalidLayerDoesNotDivideByZero) {
	const CharacterLayout layout(PixelRect{0, 0, 128, 128}, 64, 64);

	const PixelRect noCols = layout.layerRect(makeLayer(0, 1), makeLayer(1, 2));
	EXPECT_LE(noCols.width, 0);
	EXPECT_LE(noCols.height, 0);

	const PixelRect noRows = layout.layerRect(makeLayer(1, -1), makeLayer(1, 2));
	EXPECT_LE(noRows.width, 0);
	EXPECT_LE(noRows.height, 0);
}

}  // namespace
