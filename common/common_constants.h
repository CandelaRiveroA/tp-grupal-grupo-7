#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include <cstdint>

// Nuevos enumeradores para el Tablero
enum class Direction : uint8_t { 
    UP = 0, 
    DOWN = 1, 
    LEFT = 2, 
    RIGHT = 3 
};

// Como acordamos, usamos HAND para levantar objetos
enum class ToolType : uint8_t { 
    HAND = 0,
    PICKAXE = 1, 
    SHOVEL = 2, 
    WATERING_CAN = 3, 
    SWORD = 4 
};

#endif // COMMON_CONSTANTS_H
