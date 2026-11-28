#ifndef COMMUNITY_CENTER_H
#define COMMUNITY_CENTER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../inventory/item_types.h"

enum class CommunitySet : uint8_t {
    CROPS = 1,
    MINERALS = 2,
    ANIMALS = 3,
    COMBAT = 4,
    MONEY = 5
};

struct CommunityGoal {
    ItemID item;
    std::string id;
    uint16_t current;
    uint16_t total;
};

class CommunityCenter {
private:
    static const std::unordered_set<ItemID> kCropItems;
    static const std::unordered_set<ItemID> kMineralItems;
    static const std::unordered_set<ItemID> kAnimalItems;

    std::unordered_map<uint8_t, bool> completed;
    std::unordered_map<ItemID, uint32_t> crop_donations;
    std::unordered_map<ItemID, uint32_t> mineral_donations;
    std::unordered_map<ItemID, uint32_t> animal_donations;
    std::unordered_map<uint16_t, uint16_t> orc_kills_by_group;
    std::unordered_map<uint16_t, uint16_t> vampire_kills_by_pair;
    uint16_t consecutive_skeleton_rooms;
    uint16_t last_skeleton_room = 65535;
    bool skull_survived_without_damage;
    uint32_t gold_donated;
    std::vector<uint8_t> completed_sets;

public:
    CommunityCenter();
    std::vector<CommunityGoal> goals_for(CommunitySet set) const;

    bool offer_crop(ItemID item, Quality quality, uint16_t quantity);
    bool offer_mineral(ItemID item, uint16_t quantity);
    bool offer_animal(ItemID item, uint16_t quantity);
    bool offer_gold(uint16_t amount);
    bool is_valid_item(CommunitySet set, ItemID item) const;
    void record_enemy_killed(const std::string& kind, uint16_t group_id,
                             uint16_t pair_id, uint16_t room);
    void record_skull_survival(bool survived_without_damage);

    bool try_complete_if_ready(CommunitySet set);
    bool complete_set(CommunitySet set);
    bool complete_all_sets();
    bool is_set_completed(CommunitySet set) const;
    bool is_community_center_completed() const;
    uint32_t get_gold_donated() const;
    std::vector<uint8_t> take_completed_sets();
};

#endif  // COMMUNITY_CENTER_H
