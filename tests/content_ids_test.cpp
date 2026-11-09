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

TEST(ContentIdsTest, ReverseLookupFindsItemKinds) {
    // La traducción inversa es sobre las MISMAS tablas: es la que usan
    // las fuentes de estado para equipar lo que hay en un slot.
    const ContentIds ids;
    EXPECT_EQ(ids.itemKindFor("shovel"), ItemKind::Shovel);
    EXPECT_EQ(ids.itemKindFor("milk"), ItemKind::Milk);
    // Un recurso no es un ítem equipable; un desconocido tampoco.
    EXPECT_EQ(ids.itemKindFor("silver"), std::nullopt);
    EXPECT_EQ(ids.itemKindFor("nope"), std::nullopt);
}

TEST(ContentIdsTest, ReverseLookupFindsResourceKinds) {
    const ContentIds ids;
    EXPECT_EQ(ids.resourceKindFor("coal"), ResourceKind::Coal);
    EXPECT_EQ(ids.resourceKindFor("pickaxe"), std::nullopt);
    EXPECT_EQ(ids.resourceKindFor("nope"), std::nullopt);
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
