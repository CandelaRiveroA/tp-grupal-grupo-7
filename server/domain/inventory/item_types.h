#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H

#include <cstdint>

enum class ItemID : uint16_t {
    NONE,
    PICKAXE,
    HAND,
    BED,
    SHOVEL,
    WATERING_CAN,
    SWORD,
    STRAWBERRY,
    GREEN_ONION,
    POTATO,
    PUMPKIN,
    STRAWBERRY_SEED, 
    ONION_SEED, 
    POTATO_SEED, 
    PUMPKIN_SEED,
    MILK,
    EGG,
    COAL_ORB,
    LAPIS_ORB,
    SILVER_ORB,
    GOLD_ORB,
    ROCK,
    ROCK_2,
    ROCK_3,
    ROCK_4,
    ROCK_5,
    ROCK_6,
    CHESTS
};

enum class Quality : uint8_t { BASE = 1, GOOD = 2, EXCELLENT = 3, INCREDIBLE = 4, PERFECT = 5 };

#endif  // ITEM_TYPES_H
