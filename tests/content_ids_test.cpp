#include "world/ContentIds.h"

#include "gtest/gtest.h"

namespace {

using world::ContentIds;
using world::ItemKind;
using world::ResourceKind;

TEST(ContentIdsTest, FindsItemContentIds) {
    const ContentIds ids;
    ASSERT_NE(ids.find(ItemKind::Egg), nullptr);
    EXPECT_EQ(*ids.find(ItemKind::Egg), "egg");
    EXPECT_EQ(*ids.find(ItemKind::WateringCan), "watering_can");
}

TEST(ContentIdsTest, FindsResourceContentIds) {
    const ContentIds ids;
    ASSERT_NE(ids.find(ResourceKind::Silver), nullptr);
    EXPECT_EQ(*ids.find(ResourceKind::Silver), "silver");
}

TEST(ContentIdsTest, CoversEveryItemKind) {
    // El vocabulario es total: ningún ítem queda sin contentId (las
    // tablas de los TOML se recorren con este vocabulario).
    const ContentIds ids;
    for (const auto& [kind, contentId] : ids.itemEntries()) {
        EXPECT_NE(ids.find(kind), nullptr);
        EXPECT_FALSE(contentId.empty());
    }
    EXPECT_EQ(ids.itemEntries().size(), 6u);
    EXPECT_EQ(ids.resourceEntries().size(), 4u);
}

}  // namespace
