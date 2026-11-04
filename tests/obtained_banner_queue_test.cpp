#include <vector>

#include "game/LocalWorldView.h"
#include "game/ObtainedBannerQueue.h"
#include "gtest/gtest.h"
#include "world/ResourceCatalog.h"

namespace {

using game::ActiveBanner;
using game::ObtainedBannerQueue;

TEST(ObtainedBannerQueueTest, StartsEmpty) {
    ObtainedBannerQueue queue(120, 4);
    EXPECT_TRUE(queue.active().empty());
}

TEST(ObtainedBannerQueueTest, PushAddsBannerWithFullDuration) {
    ObtainedBannerQueue queue(120, 4);
    queue.push("gold");
    ASSERT_EQ(queue.active().size(), 1u);
    EXPECT_EQ(queue.active()[0].contentId, "gold");
    EXPECT_EQ(queue.active()[0].remainingTicks, 120);
    EXPECT_EQ(queue.active()[0].stackIndex, 0);
}

TEST(ObtainedBannerQueueTest, TickConsumesAndExpires) {
    ObtainedBannerQueue queue(3, 4);
    queue.push("silver");
    queue.tick();
    EXPECT_EQ(queue.active()[0].remainingTicks, 2);
    queue.tick();
    queue.tick();
    EXPECT_TRUE(queue.active().empty());
}

TEST(ObtainedBannerQueueTest, MultiplePushesStackWithIncreasingIndex) {
    ObtainedBannerQueue queue(120, 4);
    queue.push("silver");
    queue.push("coal");
    queue.push("gold");
    ASSERT_EQ(queue.active().size(), 3u);
    EXPECT_EQ(queue.active()[0].stackIndex, 0);
    EXPECT_EQ(queue.active()[1].stackIndex, 1);
    EXPECT_EQ(queue.active()[2].stackIndex, 2);
    EXPECT_EQ(queue.active()[2].contentId, "gold");
}

TEST(ObtainedBannerQueueTest, RespectsMaxVisible) {
    ObtainedBannerQueue queue(120, 4);
    for (int i = 0; i < 6; ++i) {
        queue.push("coal");
    }
    EXPECT_EQ(queue.active().size(), 4u);
}

TEST(ObtainedBannerQueueTest, ExpiredTopRefluxesIndices) {
    // El primero vive menos: al expirar, los de abajo suben (reflujo).
    ObtainedBannerQueue queue(10, 4);
    queue.push("silver");  // morirá primero
    // Avanzamos para que el primero quede con poca vida y agregamos otro.
    for (int i = 0; i < 9; ++i) {
        queue.tick();
    }
    queue.push("gold");  // recién agregado, vida completa
    ASSERT_EQ(queue.active().size(), 2u);
    // Un tick más mata al primero (Silver).
    queue.tick();
    ASSERT_EQ(queue.active().size(), 1u);
    EXPECT_EQ(queue.active()[0].contentId, "gold");
    EXPECT_EQ(queue.active()[0].stackIndex, 0);  // recompactado al tope
}

TEST(ObtainedBannerQueueTest, ObserveAddsOnePerEvent) {
    ObtainedBannerQueue queue(120, 4);
    const std::vector<game::ViewObtained> events = {
            {"silver"},
            {"gold"},
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
