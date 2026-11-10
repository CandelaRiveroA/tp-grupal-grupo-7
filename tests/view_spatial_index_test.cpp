#include <cstddef>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "game/ViewSpatialIndex.h"

namespace {

using game::LocalWorldView;
using game::ViewDroppedItem;
using game::ViewEntity;
using game::ViewResource;
using game::ViewSpatialIndex;
using world::CreatureKind;
using world::Direction;
using world::PixelRect;
using world::ResourceState;
using world::ScenePixelMapper;

TEST(ViewSpatialIndexTest, QueriesEachCollectionIndependently) {
	LocalWorldView view;
	view.resources.push_back(
		ViewResource{"coal", 0, 0, ResourceState::Intact});
	view.droppedItems.push_back(ViewDroppedItem{"coal", 50, 50});
	ViewEntity entity{};
	entity.kind = CreatureKind::Player;
	entity.cellX = 0;
	entity.cellY = 0;
	entity.facing = Direction::Front;
	entity.moving = false;
	view.entities.push_back(entity);

	ViewSpatialIndex index;
	index.rebuild(view, ScenePixelMapper{32});

	std::vector<std::uint32_t> visible;
	index.queryResources(PixelRect{0, 0, 64, 64}, visible);
	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 0u);

	index.queryDroppedItems(PixelRect{0, 0, 64, 64}, visible);
	EXPECT_TRUE(visible.empty());

	index.queryEntities(PixelRect{0, 0, 64, 64}, visible);
	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 0u);
}

TEST(ViewSpatialIndexTest, RebuildDropsOldSnapshotEntries) {
	LocalWorldView view;
	view.resources.push_back(
		ViewResource{"coal", 0, 0, ResourceState::Intact});

	ViewSpatialIndex index;
	index.rebuild(view, ScenePixelMapper{32});

	view.resources.clear();
	index.rebuild(view, ScenePixelMapper{32});

	std::vector<std::uint32_t> visible;
	index.queryResources(PixelRect{0, 0, 64, 64}, visible);
	EXPECT_TRUE(visible.empty());
}

}  // namespace
