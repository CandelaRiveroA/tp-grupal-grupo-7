#include "community_center.h"

#include <algorithm>
#include <functional>
#include <map>
#include <unordered_map>

namespace {

using Donations = std::unordered_map<ItemID, uint32_t>;

uint16_t capped_count(const Donations& donations, ItemID item, uint16_t total) {
    const auto it = donations.find(item);
    return static_cast<uint16_t>(it == donations.end() ? 0u :
            std::min<uint32_t>(it->second, total));
}

CommunityGoal make_goal(const Donations& donations, ItemID item, const char* id, uint16_t total) {
    return CommunityGoal{item, id, capped_count(donations, item, total), total};
}

}  // namespace

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
}

void CommunityCenter::record_skull_survival(bool survived_without_damage) {
    skull_survived_without_damage = survived_without_damage;
}

bool CommunityCenter::try_complete_if_ready(CommunitySet set) {
    const auto goals = goals_for(set);
    return !goals.empty() && std::all_of(goals.begin(), goals.end(), [](const CommunityGoal& goal) {
        return goal.current >= goal.total;
    });
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

std::vector<CommunityGoal> CommunityCenter::goals_for(CommunitySet set) const {
    using Maker = std::vector<CommunityGoal> (*)(const CommunityCenter&);
    static const std::map<CommunitySet, Maker> handlers = {
        {CommunitySet::CROPS, [](const CommunityCenter& center) -> std::vector<CommunityGoal> {
            return {
                make_goal(center.crop_donations, ItemID::STRAWBERRY, "strawberry", 3),
                make_goal(center.crop_donations, ItemID::GREEN_ONION, "onion", 3),
                make_goal(center.crop_donations, ItemID::POTATO, "potato", 3),
                make_goal(center.crop_donations, ItemID::PUMPKIN, "pumpkin", 3)
            };
        }},
        {CommunitySet::MINERALS, [](const CommunityCenter& center) -> std::vector<CommunityGoal> {
            return {
                make_goal(center.mineral_donations, ItemID::COAL_ORB, "coal_orb", 10),
                make_goal(center.mineral_donations, ItemID::LAPIS_ORB, "lapis_orb", 7),
                make_goal(center.mineral_donations, ItemID::SILVER_ORB, "silver_orb", 5),
                make_goal(center.mineral_donations, ItemID::GOLD_ORB, "gold_orb", 3)
            };
        }},
        {CommunitySet::ANIMALS, [](const CommunityCenter& center) -> std::vector<CommunityGoal> {
            return {
                make_goal(center.animal_donations, ItemID::MILK, "milk", 5),
                make_goal(center.animal_donations, ItemID::EGG, "egg", 10)
            };
        }},
        {CommunitySet::MONEY, [](const CommunityCenter& center) -> std::vector<CommunityGoal> {
            return {
                {ItemID::NONE, "money",
                 static_cast<uint16_t>(std::min<uint32_t>(center.gold_donated, 10000)),
                 10000}
            };
        }},
        {CommunitySet::COMBAT, [](const CommunityCenter& center) -> std::vector<CommunityGoal> {
            uint16_t orcs = 0;
            uint16_t vampires = 0;

            for (const auto& entry: center.orc_kills_by_group) {
                orcs = std::max(orcs, entry.second);
            }
            for (const auto& entry: center.vampire_kills_by_pair) {
                vampires = std::max(vampires, entry.second);
            }

            return {
                {ItemID::NONE, "skeleton",
                 std::min<uint16_t>(center.consecutive_skeleton_rooms, 3), 3},
                {ItemID::NONE, "vampire", std::min<uint16_t>(vampires, 2), 2},
                {ItemID::NONE, "orc", std::min<uint16_t>(orcs, 4), 4},
                {ItemID::NONE, "skull",
                 static_cast<uint16_t>(center.skull_survived_without_damage), 1}
            };
        }}
    };

    const auto it = handlers.find(set);
    if (it == handlers.end()) {
        return {};
    }
    return it->second(*this);
}
