#include "gtest/gtest.h"

#include "sdl/render/Camera.h"

namespace {

using sdl::Camera;
using world::PixelRect;

TEST(CameraTest, StartsAtTheWorldOrigin) {
	const Camera camera(PixelRect{10, 20, 1000, 1000}, 200, 200);
	EXPECT_EQ(camera.offsetX(), 10);
	EXPECT_EQ(camera.offsetY(), 20);
}

TEST(CameraTest, CentersTheTargetInTheViewport) {
	Camera camera(PixelRect{0, 0, 1000, 1000}, 200, 200);
	camera.follow(PixelRect{480, 480, 40, 40});

	EXPECT_EQ(camera.offsetX(), 400);
	EXPECT_EQ(camera.offsetY(), 400);

	const PixelRect screen =
		camera.viewport().toScreen(PixelRect{480, 480, 40, 40});
	EXPECT_EQ(screen.x, 80);
	EXPECT_EQ(screen.y, 80);
	EXPECT_EQ(screen.width, 40);
	EXPECT_EQ(screen.height, 40);
}

TEST(CameraTest, ClampsAtTheLeftAndTopEdges) {
	Camera camera(PixelRect{0, 0, 1000, 1000}, 200, 200);
	camera.follow(PixelRect{0, 0, 40, 40});

	EXPECT_EQ(camera.offsetX(), 0);
	EXPECT_EQ(camera.offsetY(), 0);
}

TEST(CameraTest, ClampsAtTheRightAndBottomEdges) {
	Camera camera(PixelRect{0, 0, 1000, 1000}, 200, 200);
	camera.follow(PixelRect{960, 960, 40, 40});

	EXPECT_EQ(camera.offsetX(), 800);
	EXPECT_EQ(camera.offsetY(), 800);

	const PixelRect screen =
		camera.viewport().toScreen(PixelRect{960, 960, 40, 40});
	EXPECT_EQ(screen.x, 160);
	EXPECT_EQ(screen.y, 160);
}

TEST(CameraTest, KeepsTheWorldStillWhenItIsSmallerThanTheViewport) {
	Camera camera(PixelRect{0, 0, 100, 80}, 200, 200);
	camera.follow(PixelRect{20, 10, 40, 40});

	EXPECT_EQ(camera.offsetX(), 0);
	EXPECT_EQ(camera.offsetY(), 0);
}

TEST(CameraTest, UsesTheWorldOriginForTheClampRange) {
	Camera camera(PixelRect{10, 20, 1000, 1000}, 200, 200);
	camera.follow(PixelRect{0, 0, 40, 40});

	EXPECT_EQ(camera.offsetX(), 10);
	EXPECT_EQ(camera.offsetY(), 20);
}

TEST(CameraTest, HandlesAnOddViewportSize) {
	Camera camera(PixelRect{0, 0, 1000, 1000}, 101, 101);
	camera.follow(PixelRect{484, 484, 32, 32});

	EXPECT_EQ(camera.offsetX(), 450);
	EXPECT_EQ(camera.offsetY(), 450);
}

TEST(CameraTest, TranslatesARectangleByTheCurrentOffset) {
	Camera camera(PixelRect{0, 0, 1000, 1000}, 200, 200);
	camera.follow(PixelRect{480, 480, 40, 40});

	const PixelRect screen =
		camera.viewport().toScreen(PixelRect{100, 50, 20, 10});
	EXPECT_EQ(screen.x, -300);
	EXPECT_EQ(screen.y, -350);
	EXPECT_EQ(screen.width, 20);
	EXPECT_EQ(screen.height, 10);
}

}
