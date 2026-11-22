#include "community_set_rewards.h"

#include <memory>

#include "../animals/chicken.h"
#include "../animals/cow.h"
#include "../inventory/product.h"
#include "../map/world.h"

CommunitySetRewards::CommunitySetRewards():
        rewards{
                {CommunitySet::CROPS, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        Player& player = world.get_player(id);
                        for (int i = 0; i < 5; ++i) {
                            player.pick_up_item(std::make_unique<Product>(
                                    ItemID::STRAWBERRY_SEED, 20, Quality::BASE));
                            player.pick_up_item(std::make_unique<Product>(
                                    ItemID::ONION_SEED, 20, Quality::BASE));
                            player.pick_up_item(std::make_unique<Product>(
                                    ItemID::POTATO_SEED, 20, Quality::BASE));
                            player.pick_up_item(std::make_unique<Product>(
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
                        world.add_animal(std::make_unique<Cow>(
                                player.get_x() + 1 < world.get_width()
                                        ? player.get_x() + 1
                                        : player.get_x(),
                                player.get_y()));
                        world.add_animal(std::make_unique<Chicken>(
                                player.get_x(),
                                player.get_y() + 1 < world.get_height()
                                        ? player.get_y() + 1
                                        : player.get_y()));
                        world.add_animal(std::make_unique<Chicken>(
                                player.get_x() + 1 < world.get_width()
                                        ? player.get_x() + 1
                                        : player.get_x(),
                                player.get_y() + 1 < world.get_height()
                                        ? player.get_y() + 1
                                        : player.get_y()));
                    }
                }},
                {CommunitySet::COMBAT, [](World& world) {
                    for (uint8_t id: world.get_player_ids()) {
                        Player& player = world.get_player(id);
                        const uint16_t max_health =
                                player.get_max_health() + player.get_max_health() / 4;
                        player.set_max_health(max_health);
                        player.set_health(player.get_health() + player.get_health() / 4);
                        const float max_energy = player.get_max_energy() * 1.25f;
                        player.set_max_energy(max_energy);
                        player.set_energy(player.get_energy() * 1.25f);
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
