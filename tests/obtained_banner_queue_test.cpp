#include <vector>

#include "game/LocalWorldView.h"
#include "game/ObtainedBannerQueue.h"
#include "gtest/gtest.h"
#include "world/ResourceCatalog.h"

namespace {

using game::ActiveBanner;
using game::ObtainedBannerQueue;
using world::ResourceKind;

TEST(ObtainedBannerQueueTest, StartsEmpty) {
    ObtainedBannerQueue queue(120, 4);
    EXPECT_TRUE(queue.active().empty());
}

TEST(ObtainedBannerQueueTest, PushAddsBannerWithFullDuration) {
    ObtainedBannerQueue queue(120, 4);
    queue.push(ResourceKind::Gold);
    ASSERT_EQ(queue.active().size(), 1u);
    EXPECT_EQ(queue.active()[0].kind, ResourceKind::Gold);
    EXPECT_EQ(queue.active()[0].remainingTicks, 120);
    EXPECT_EQ(queue.active()[0].stackIndex, 0);
}

TEST(ObtainedBannerQueueTest, TickConsumesAndExpires) {
    ObtainedBannerQueue queue(3, 4);
    queue.push(ResourceKind::Silver);
    queue.tick();
    EXPECT_EQ(queue.active()[0].remainingTicks, 2);
    queue.tick();
    queue.tick();
    EXPECT_TRUE(queue.active().empty());
}

TEST(ObtainedBannerQueueTest, MultiplePushesStackWithIncreasingIndex) {
    ObtainedBannerQueue queue(120, 4);
    queue.push(ResourceKind::Silver);
    queue.push(ResourceKind::Coal);
    queue.push(ResourceKind::Gold);
    ASSERT_EQ(queue.active().size(), 3u);
    EXPECT_EQ(queue.active()[0].stackIndex, 0);
    EXPECT_EQ(queue.active()[1].stackIndex, 1);
    EXPECT_EQ(queue.active()[2].stackIndex, 2);
    EXPECT_EQ(queue.active()[2].kind, ResourceKind::Gold);
}

TEST(ObtainedBannerQueueTest, RespectsMaxVisible) {
    ObtainedBannerQueue queue(120, 4);
    for (int i = 0; i < 6; ++i) {
        queue.push(ResourceKind::Coal);
    }
    EXPECT_EQ(queue.active().size(), 4u);
}

TEST(ObtainedBannerQueueTest, ExpiredTopRefluxesIndices) {
    // El primero vive menos: al expirar, los de abajo suben (reflujo).
    ObtainedBannerQueue queue(10, 4);
    queue.push(ResourceKind::Silver);  // morirá primero
    // Avanzamos para que el primero quede con poca vida y agregamos otro.
    for (int i = 0; i < 9; ++i) {
        queue.tick();
    }
    queue.push(ResourceKind::Gold);  // recién agregado, vida completa
    ASSERT_EQ(queue.active().size(), 2u);
    // Un tick más mata al primero (Silver).
    queue.tick();
    ASSERT_EQ(queue.active().size(), 1u);
    EXPECT_EQ(queue.active()[0].kind, ResourceKind::Gold);
    EXPECT_EQ(queue.active()[0].stackIndex, 0);  // recompactado al tope
}

TEST(ObtainedBannerQueueTest, ObserveAddsOnePerEvent) {
    ObtainedBannerQueue queue(120, 4);
    const std::vector<game::ViewResourceObtained> events = {
            {ResourceKind::Silver},
            {ResourceKind::Gold},
    };
    queue.observe(events);
    EXPECT_EQ(queue.active().size(), 2u);
}

TEST(ObtainedBannerQueueTest, ObserveEmptyDoesNothing) {
    ObtainedBannerQueue queue(120, 4);
    queue.observe({});
    EXPECT_TRUE(queue.active().empty());
}

}  // namespace
