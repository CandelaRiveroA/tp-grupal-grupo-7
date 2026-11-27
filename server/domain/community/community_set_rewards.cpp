#include "community_set_rewards.h"

#include <memory>

#include "../inventory/product.h"
#include "../map/world.h"

CommunitySetRewards::CommunitySetRewards():
        rewards{
                {CommunitySet::CROPS, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        Player& player = world.get_player(id);
                        for (int i = 0; i < 5; ++i) {
                            world.give_item_or_gold(player, std::make_unique<Product>(
                                    ItemID::STRAWBERRY_SEED, 20, Quality::BASE));
                            world.give_item_or_gold(player, std::make_unique<Product>(
                                    ItemID::ONION_SEED, 20, Quality::BASE));
                            world.give_item_or_gold(player, std::make_unique<Product>(
                                    ItemID::POTATO_SEED, 20, Quality::BASE));
                            world.give_item_or_gold(player, std::make_unique<Product>(
                                    ItemID::PUMPKIN_SEED, 20, Quality::BASE));
                        }
                    }
                }},
                {CommunitySet::MINERALS, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        world.get_player(id).add_gold(500);
                    }
                }},
                {CommunitySet::ANIMALS, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        Player& player = world.get_player(id);
                        world.reward_animals(player, ItemID::COW, 1);
                        world.reward_animals(player, ItemID::CHICKEN, 2);
                    }
                }},
                {CommunitySet::COMBAT, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        Player& player = world.get_player(id);
                        const uint16_t max_health =
                                player.get_max_health() + player.get_max_health() / 4;
                        player.set_max_health(max_health);
                        player.set_health(player.get_health() + player.get_health() / 4);
                        const uint32_t max_energy =
                                static_cast<uint32_t>(player.get_max_energy()) * 125u / 100u;
                        player.set_max_energy(static_cast<uint16_t>(max_energy));
                        const uint32_t energy =
                                static_cast<uint32_t>(player.get_energy()) * 125u / 100u;
                        player.set_energy(static_cast<uint16_t>(energy));
                    }
                }},
                {CommunitySet::MONEY, [](World& world) {
                    world.activate_market_rewards();
                }}} {}

void CommunitySetRewards::apply(World& world, CommunitySet set) const {
    const auto it = rewards.find(set);
    if (it != rewards.end()) {
        it->second(world);
    }
}
