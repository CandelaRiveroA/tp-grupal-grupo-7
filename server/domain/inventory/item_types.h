#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H

#include <cstdint>

enum class ItemID : uint16_t {
    PICKAXE, SHOVEL, WATERING_CAN, SWORD, BED,
    STRAWBERRY, ONION, POTATO, PUMPKIN,
    COAL_ORB, LAPIS_ORB, SILVER_ORB, GOLD_ORB
};

enum class Quality : uint8_t {
    BASE = 1, GOOD = 2, EXCELLENT = 3, INCREDIBLE = 4, PERFECT = 5
};

#endif // ITEM_TYPES_H
