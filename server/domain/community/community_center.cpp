#include "community_center.h"

#include <map>
#include <functional>



const std::unordered_set<ItemID> CommunityCenter::kCropItems = {
        ItemID::STRAWBERRY,
        ItemID::GREEN_ONION,
        ItemID::POTATO,
        ItemID::PUMPKIN};

const std::unordered_set<ItemID> CommunityCenter::kMineralItems = {
        ItemID::COAL_ORB,
        ItemID::LAPIS_ORB,
        ItemID::SILVER_ORB,
        ItemID::GOLD_ORB};

const std::unordered_set<ItemID> CommunityCenter::kAnimalItems = {
        ItemID::MILK,
        ItemID::EGG};

CommunityCenter::CommunityCenter():
        consecutive_skeleton_rooms(0),
        last_skeleton_room(65535),
        skull_survived_without_damage(false),
        gold_donated(0) {}

bool CommunityCenter::offer_crop(ItemID item, Quality quality, uint16_t quantity) {
    if (quality < Quality::INCREDIBLE) {
        return false;
    }
    if (kCropItems.find(item) == kCropItems.end()) {
        return false;
    }
    crop_donations[item] += quantity;
    return true;
}

bool CommunityCenter::offer_mineral(ItemID item, uint16_t quantity) {
    if (kMineralItems.find(item) == kMineralItems.end()) {
        return false;
    }
    mineral_donations[item] += quantity;
    return true;
}

bool CommunityCenter::offer_animal(ItemID item, uint16_t quantity) {
    if (kAnimalItems.find(item) == kAnimalItems.end()) {
        return false;
    }
    animal_donations[item] += quantity;
    return true;
}

bool CommunityCenter::is_valid_item(CommunitySet set, ItemID item) const {
    const std::map<CommunitySet, std::function<bool()>> validators = {
        {CommunitySet::CROPS, [&]() {
            return kCropItems.find(item) != kCropItems.end();
        }},
        {CommunitySet::MINERALS, [&]() {
            return kMineralItems.find(item) != kMineralItems.end();
        }},
        {CommunitySet::ANIMALS, [&]() {
            return kAnimalItems.find(item) != kAnimalItems.end();
        }},
        {CommunitySet::MONEY, [&]() {
            return item == ItemID::GOLD_ORB;
        }}
    };

    const auto it = validators.find(set);
    if (it == validators.end()) {
        return false;
    }

    return it->second();
}

bool CommunityCenter::offer_gold(uint16_t amount) {
    if (amount == 0) {
        return false;
    }
    gold_donated += amount;
    return true;
}

void CommunityCenter::record_enemy_killed(const std::string& kind, uint16_t group_id,
                                          uint16_t pair_id, uint16_t room) {
    if (kind == "skeleton") {
        if (room == 0) {
            try_complete_combat();
            return;
        }
        if (last_skeleton_room == 65535 || room == last_skeleton_room + 1) {
            ++consecutive_skeleton_rooms;
        } else if (room != last_skeleton_room) {
            consecutive_skeleton_rooms = 1;
        }
        last_skeleton_room = room;
    } else if (kind == "orc" && group_id != 0) {
        ++orc_kills_by_group[group_id];
    } else if ((kind == "vampire" || kind == "vampiress") && pair_id != 0) {
        ++vampire_kills_by_pair[pair_id];
    }
    try_complete_combat();
}

void CommunityCenter::record_skull_survival(bool survived_without_damage) {
    skull_survived_without_damage = survived_without_damage;
    try_complete_combat();
}

bool CommunityCenter::try_complete_crops() {
    return crop_donations[ItemID::STRAWBERRY] >= 3 &&
           crop_donations[ItemID::GREEN_ONION] >= 3 &&
           crop_donations[ItemID::POTATO] >= 3 &&
           crop_donations[ItemID::PUMPKIN] >= 3;
}

bool CommunityCenter::try_complete_minerals() {
    return mineral_donations[ItemID::COAL_ORB] >= 10 &&
           mineral_donations[ItemID::LAPIS_ORB] >= 7 &&
           mineral_donations[ItemID::SILVER_ORB] >= 5 &&
           mineral_donations[ItemID::GOLD_ORB] >= 3;
}

bool CommunityCenter::try_complete_animals() {
    return animal_donations[ItemID::MILK] >= 5 &&
           animal_donations[ItemID::EGG] >= 10;
}

bool CommunityCenter::try_complete_combat() {
    bool orcs_ready = false;
    for (const auto& [group, kills]: orc_kills_by_group) {
        (void)group;
        if (kills >= 4) {
            orcs_ready = true;
        }
    }
    bool vampires_ready = false;
    for (const auto& [pair, kills]: vampire_kills_by_pair) {
        (void)pair;
        if (kills >= 2) {
            vampires_ready = true;
        }
    }
    return consecutive_skeleton_rooms >= 3 && orcs_ready && vampires_ready &&
           skull_survived_without_damage;
}

bool CommunityCenter::try_complete_money() {
    return gold_donated >= 10000;
}

bool CommunityCenter::try_complete_if_ready(CommunitySet set) {
    using CompleteMethod = bool (CommunityCenter::*)();

    static const std::map<CommunitySet, CompleteMethod> methods = {
        {CommunitySet::CROPS, &CommunityCenter::try_complete_crops},
        {CommunitySet::MINERALS, &CommunityCenter::try_complete_minerals},
        {CommunitySet::ANIMALS, &CommunityCenter::try_complete_animals},
        {CommunitySet::COMBAT, &CommunityCenter::try_complete_combat},
        {CommunitySet::MONEY, &CommunityCenter::try_complete_money}
    };

    const auto it = methods.find(set);
    if (it == methods.end()) {
        return false;
    }

    return (this->*(it->second))();
}

bool CommunityCenter::complete_set(CommunitySet set) {
    const uint8_t id = static_cast<uint8_t>(set);
    if (completed[id]) {
        return false;
    }
    completed[id] = true;
    completed_sets.push_back(id);
    return true;
}

bool CommunityCenter::complete_all_sets() {
    bool changed = false;
    for (uint8_t id = 1; id <= 5; ++id) {
        changed = complete_set(static_cast<CommunitySet>(id)) || changed;
    }
    return changed;
}

bool CommunityCenter::is_set_completed(CommunitySet set) const {
    const auto it = completed.find(static_cast<uint8_t>(set));
    return it != completed.end() && it->second;
}

bool CommunityCenter::is_community_center_completed() const {
    for (uint8_t id = 1; id <= 5; ++id) {
        if (!is_set_completed(static_cast<CommunitySet>(id))) {
            return false;
        }
    }
    return true;
}

uint32_t CommunityCenter::get_gold_donated() const { return gold_donated; }

std::vector<uint8_t> CommunityCenter::take_completed_sets() {
    std::vector<uint8_t> taken;
    taken.swap(completed_sets);
    return taken;
}
