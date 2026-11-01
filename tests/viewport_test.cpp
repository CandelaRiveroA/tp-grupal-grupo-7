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

TEST(ViewportTest, ToWorldIsTheInverseOfToScreen) {
	const Viewport viewport(100, 50, 200, 100);

	// Un rect de mundo aparece desplazado por el offset al dibujarse...
	const PixelRect worldRect{130, 70, 10, 10};
	const PixelRect screen = viewport.toScreen(worldRect);
	EXPECT_EQ(screen.x, 30);
	EXPECT_EQ(screen.y, 20);

	// ...y un punto de pantalla vuelve a su lugar con toWorld.
	const world::PixelPoint worldPoint =
		viewport.toWorld(world::PixelPoint{screen.x, screen.y});
	EXPECT_EQ(worldPoint.x, 130);
	EXPECT_EQ(worldPoint.y, 70);
}

}  // namespace
