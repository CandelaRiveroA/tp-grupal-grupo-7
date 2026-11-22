#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "world/DepthOrder.h"

namespace {

std::vector<std::uint32_t> indices(const std::vector<world::DepthItem>& items) {
	std::vector<std::uint32_t> result;
	for (const world::DepthItem& item : items) {
		result.push_back(item.index);
	}
	return result;
}

TEST(DepthOrderTest, WithoutElementsEntitiesKeepTheirArrivalOrder) {
	world::DepthOrder order;
	order.addEntity(5, 0);
	order.addEntity(2, 1);
	order.addEntity(9, 2);
	EXPECT_FALSE(order.hasElements());
	EXPECT_EQ(indices(order.sorted()), (std::vector<std::uint32_t>{0, 1, 2}));
}

TEST(DepthOrderTest, PlayerBehindATreeIsDrawnBeforeIt) {
	world::DepthOrder order;
	// Árbol con la base en la fila 6; el jugador en la fila 4 (detrás).
	order.addElement(6, 7);
	order.addEntity(4, 0);
	const std::vector<world::DepthItem>& items = order.sorted();
	ASSERT_EQ(items.size(), 2u);
	EXPECT_EQ(items[0].kind, world::DepthKind::Entity);
	EXPECT_EQ(items[1].kind, world::DepthKind::Element);
}

TEST(DepthOrderTest, PlayerInFrontOfATreeIsDrawnAfterIt) {
	world::DepthOrder order;
	order.addElement(6, 7);
	order.addEntity(8, 0);
	const std::vector<world::DepthItem>& items = order.sorted();
	EXPECT_EQ(items[0].kind, world::DepthKind::Element);
	EXPECT_EQ(items[1].kind, world::DepthKind::Entity);
}

TEST(DepthOrderTest, SameRowDrawsTheElementFirst) {
	world::DepthOrder order;
	order.addEntity(6, 0);
	order.addElement(6, 7);
	const std::vector<world::DepthItem>& items = order.sorted();
	EXPECT_EQ(items[0].kind, world::DepthKind::Element);
	EXPECT_EQ(items[1].kind, world::DepthKind::Entity);
}

TEST(DepthOrderTest, ElementsKeepTheirSceneOrderWithinTheSameRow) {
	world::DepthOrder order;
	order.addElement(3, 10);
	order.addElement(3, 11);
	order.addElement(3, 12);
	EXPECT_EQ(indices(order.sorted()), (std::vector<std::uint32_t>{10, 11, 12}));
}

TEST(DepthOrderTest, MixedItemsAreSortedByRow) {
	world::DepthOrder order;
	order.addEntity(9, 1);
	order.addElement(2, 20);
	order.addEntity(4, 0);
	order.addElement(7, 21);
	EXPECT_EQ(indices(order.sorted()),
		  (std::vector<std::uint32_t>{20, 0, 21, 1}));
}

TEST(DepthOrderTest, ClearStartsAFreshFrame) {
	world::DepthOrder order;
	order.addElement(1, 1);
	order.clear();
	EXPECT_FALSE(order.hasElements());
	EXPECT_TRUE(order.sorted().empty());
}

}  // namespace
