#include <optional>

#include "gtest/gtest.h"
#include "world_model/Map.h"

namespace {

worldmodel::Map makeMap() {
	worldmodel::Map map;
	map.id = "farm";
	map.width = 4;
	map.height = 3;
	map.baseCollision.assign(12, false);
	map.baseCollision[0] = true;
	map.collision = map.baseCollision;
	map.ground.assign(12, std::nullopt);
	map.paths.assign(12, std::nullopt);
	return map;
}

worldmodel::MapObject makeObject(worldmodel::ObjectRole role,
				 worldmodel::TilePos cell, int width, int height,
				 bool blocked) {
	worldmodel::MapObject object;
	object.role = role;
	object.tile = worldmodel::TileRef{"outdoors", 0, 0};
	object.cell = cell;
	object.width = width;
	object.height = height;
	object.blocked = blocked;
	return object;
}

TEST(MapTest, ContainsChecksBounds) {
	const worldmodel::Map map = makeMap();
	EXPECT_TRUE(map.contains({0, 0}));
	EXPECT_TRUE(map.contains({3, 2}));
	EXPECT_FALSE(map.contains({-1, 0}));
	EXPECT_FALSE(map.contains({0, -1}));
	EXPECT_FALSE(map.contains({4, 0}));
	EXPECT_FALSE(map.contains({0, 3}));
}

TEST(MapTest, CellIndexIsRowMajor) {
	const worldmodel::Map map = makeMap();
	EXPECT_EQ(map.cellIndex({2, 1}), 6u);
}

TEST(MapTest, IsWalkableUsesCollisionAndBounds) {
	const worldmodel::Map map = makeMap();
	EXPECT_FALSE(map.isWalkable({0, 0}));
	EXPECT_TRUE(map.isWalkable({1, 0}));
	EXPECT_FALSE(map.isWalkable({-1, 0}));
	EXPECT_FALSE(map.isWalkable({4, 0}));
}

TEST(MapTest, IsWalkableRejectsMissingCollision) {
	worldmodel::Map map = makeMap();
	map.collision.clear();
	EXPECT_FALSE(map.isWalkable({1, 0}));
}

TEST(MapTest, ComposeAddsBlockedFootprintWithoutMutating) {
	worldmodel::Map map = makeMap();
	map.objects.push_back(makeObject(worldmodel::ObjectRole::Cabin, {1, 1},
					 2, 2, true));
	map.objects.push_back(makeObject(worldmodel::ObjectRole::Decoration,
					 {3, 0}, 1, 1, false));

	const std::vector<bool> composed = map.composeCollision();

	EXPECT_TRUE(composed[0]);
	EXPECT_TRUE(composed[1 * 4 + 1]);
	EXPECT_TRUE(composed[1 * 4 + 2]);
	EXPECT_TRUE(composed[2 * 4 + 1]);
	EXPECT_TRUE(composed[2 * 4 + 2]);
	EXPECT_FALSE(composed[3]);
	EXPECT_FALSE(map.collision[1 * 4 + 1]);
}

TEST(MapTest, RebuildCollisionAssignsComposedMask) {
	worldmodel::Map map = makeMap();
	map.objects.push_back(makeObject(worldmodel::ObjectRole::Cabin, {1, 1},
					 2, 2, true));

	map.rebuildCollision();

	EXPECT_TRUE(map.collision[1 * 4 + 1]);
	EXPECT_TRUE(map.collision[2 * 4 + 2]);
	EXPECT_FALSE(map.collision[3]);
}

TEST(MapTest, ComposeIgnoresOutOfBoundsFootprint) {
	worldmodel::Map map = makeMap();
	map.objects.push_back(makeObject(worldmodel::ObjectRole::Decoration,
					 {3, 2}, 2, 2, true));

	const std::vector<bool> composed = map.composeCollision();

	ASSERT_EQ(composed.size(), 12u);
	EXPECT_TRUE(composed[2 * 4 + 3]);
}

}
