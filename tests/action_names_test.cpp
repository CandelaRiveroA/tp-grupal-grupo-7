#include "world/ActionNames.h"

#include "gtest/gtest.h"

namespace {

using world::ActionKind;
using world::ActionNames;

TEST(ActionNamesTest, TranslatesBothWays) {
    const ActionNames names;
    const ActionKind* kind = names.find("pickup");
    ASSERT_NE(kind, nullptr);
    EXPECT_EQ(*kind, ActionKind::Pickup);

    const std::string* name = names.nameFor(ActionKind::Milk);
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(*name, "milk");
}

TEST(ActionNamesTest, UnknownLookupsGiveNull) {
    const ActionNames names;
    EXPECT_EQ(names.find("dance"), nullptr);
    // nameFor: la búsqueda inversa es total para las acciones del
    // vocabulario; el nullptr cubre futuras acciones sin nombre TOML.
    EXPECT_NE(names.nameFor(ActionKind::Idle), nullptr);
}

}  // namespace
