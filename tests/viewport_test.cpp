#include "gtest/gtest.h"

#include "sdl/render/Viewport.h"

namespace {

using sdl::Viewport;
using world::PixelRect;

TEST(ViewportTest, ExposesScreenAndWorldRects) {
	const Viewport viewport(100, 50, 200, 100);

	EXPECT_EQ(viewport.offsetX(), 100);
	EXPECT_EQ(viewport.offsetY(), 50);
	EXPECT_EQ(viewport.width(), 200);
	EXPECT_EQ(viewport.height(), 100);

	const PixelRect screen = viewport.screenRect();
	EXPECT_EQ(screen.x, 0);
	EXPECT_EQ(screen.y, 0);
	EXPECT_EQ(screen.width, 200);
	EXPECT_EQ(screen.height, 100);

	const PixelRect world = viewport.visibleWorldRect();
	EXPECT_EQ(world.x, 100);
	EXPECT_EQ(world.y, 50);
	EXPECT_EQ(world.width, 200);
	EXPECT_EQ(world.height, 100);
}

TEST(ViewportTest, TranslatesWorldRectanglesToScreen) {
	const Viewport viewport(100, 50, 200, 100);
	const PixelRect screen =
		viewport.toScreen(PixelRect{120, 70, 20, 10});

	EXPECT_EQ(screen.x, 20);
	EXPECT_EQ(screen.y, 20);
	EXPECT_EQ(screen.width, 20);
	EXPECT_EQ(screen.height, 10);
}

TEST(ViewportTest, IntersectsOnlyWithPositiveOverlap) {
	const Viewport viewport(0, 0, 200, 100);

	EXPECT_TRUE(viewport.intersects(PixelRect{199, 99, 10, 10}));
	EXPECT_TRUE(viewport.intersects(PixelRect{0, 0, 200, 100}));
	EXPECT_FALSE(viewport.intersects(PixelRect{200, 0, 10, 10}));
	EXPECT_FALSE(viewport.intersects(PixelRect{-10, 0, 10, 10}));
	EXPECT_FALSE(viewport.intersects(PixelRect{0, 0, 0, 10}));
}

}  // namespace
