#include "gtest/gtest.h"
#include "world/ScenePixelMapper.h"

namespace {


TEST(ScenePixelMapperTest, OriginCellMapsToTopLeft) {
	const world::ScenePixelMapper mapper(32);
	const world::PixelRect rect =
		mapper.toPixels(world::GridCell{0, 0}, world::GridSpan{1, 1});
	EXPECT_EQ(rect.x, 0);
	EXPECT_EQ(rect.y, 0);
	EXPECT_EQ(rect.width, 32);
	EXPECT_EQ(rect.height, 32);
}

TEST(ScenePixelMapperTest, CellOffsetScalesByCellSize) {
	const world::ScenePixelMapper mapper(32);
	const world::PixelRect rect =
		mapper.toPixels(world::GridCell{3, 2}, world::GridSpan{1, 1});
	EXPECT_EQ(rect.x, 96);
	EXPECT_EQ(rect.y, 64);
}

TEST(ScenePixelMapperTest, SpanScalesWidthAndHeight) {
	const world::ScenePixelMapper mapper(32);
	const world::PixelRect rect = mapper.toPixels(
		world::GridCell{0, 2}, world::GridSpan{30, 18});
	EXPECT_EQ(rect.x, 0);
	EXPECT_EQ(rect.y, 64);
	EXPECT_EQ(rect.width, 960);
	EXPECT_EQ(rect.height, 576);
}

}
