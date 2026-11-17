#include "commands/chat_command.h"

#include <map>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "domain/entities/player.h"
#include "domain/map/world.h"

namespace {

std::map<uint8_t, std::string> names_with(uint8_t id, const std::string& name) {
    std::map<uint8_t, std::string> m;
    m[id] = name;
    return m;
}

}  // namespace

TEST(ChatRouterTest, PublicMessageBroadcasts) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    ChatRouter router;
    auto replies = router.route("Hola, gente!", 1, world, names_with(1, "Pandamos"));
    ASSERT_EQ(replies.size(), 1u);
    EXPECT_TRUE(replies[0].broadcast);
    const auto* chat = std::get_if<ServerChat>(&replies[0].message);
    ASSERT_NE(chat, nullptr);
    EXPECT_EQ(chat->characterName, "Pandamos");
    EXPECT_EQ(chat->text, "Hola, gente!");
    EXPECT_FALSE(chat->system);
}

TEST(ChatRouterTest, PrivateMessageDeliversToTargetAndSender) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.add_player(2, std::make_unique<Player>(2, 2, 2));
    ChatRouter router;
    std::map<uint8_t, std::string> names;
    names[1] = "Pandamos";
    names[2] = "dipa";
    auto replies = router.route("@dipa vamos a las mazmorras?", 1, world, names);
    ASSERT_EQ(replies.size(), 2u);
    for (const auto& r : replies) {
        EXPECT_FALSE(r.broadcast);
        const auto* chat = std::get_if<ServerChat>(&r.message);
        ASSERT_NE(chat, nullptr);
        EXPECT_EQ(chat->text, "@dipa vamos a las mazmorras?");
    }
    EXPECT_EQ(replies[0].target, 2u);
    EXPECT_EQ(replies[1].target, 1u);
}

TEST(ChatRouterTest, PrivateToUnknownPlayerExplains) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    ChatRouter router;
    auto replies = router.route("@fantasma hola", 1, world, names_with(1, "Pandamos"));
    ASSERT_EQ(replies.size(), 1u);
    EXPECT_FALSE(replies[0].broadcast);
    const auto* chat = std::get_if<ServerChat>(&replies[0].message);
    ASSERT_NE(chat, nullptr);
    EXPECT_TRUE(chat->system);
}

TEST(ChatRouterTest, VidaCheatSetsHealthAndReplies) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    ChatRouter router;
    auto replies = router.route("/vida 50", 1, world, names_with(1, "Pandamos"));
    ASSERT_EQ(replies.size(), 1u);
    EXPECT_FALSE(replies[0].broadcast);
    const auto* chat = std::get_if<ServerChat>(&replies[0].message);
    ASSERT_NE(chat, nullptr);
    EXPECT_TRUE(chat->system);
    EXPECT_EQ(world.get_player(1).get_health(), 50u);
}

TEST(ChatRouterTest, OroCheatFloorsAtZero) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.get_player(1).set_gold(3);
    ChatRouter router;
    auto replies = router.route("/oro -10", 1, world, names_with(1, "Pandamos"));
    ASSERT_EQ(replies.size(), 1u);
    EXPECT_EQ(world.get_player(1).get_gold(), 0u);
}

TEST(ChatRouterTest, UnknownCommandExplains) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    ChatRouter router;
    auto replies = router.route("/volar", 1, world, names_with(1, "Pandamos"));
    ASSERT_EQ(replies.size(), 1u);
    const auto* chat = std::get_if<ServerChat>(&replies[0].message);
    ASSERT_NE(chat, nullptr);
    EXPECT_TRUE(chat->system);
}

TEST(ChatContractTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(protocol::ClientMessageType::Chat), 6);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessageType::Chat), 23);
}