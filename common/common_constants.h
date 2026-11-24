#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include <cstdint>

#define kMaxChatLength 256

// Hora de juego: de las 6:00 a las 2:00 del día siguiente (20 h), en saltos de 10 min.
// Compartida entre el reloj del Match y el cheat /tiempo.
#define kDayClockMinutes 1200

// Nuevos enumeradores para el Tablero
enum class Direction : uint8_t { 
    UP = 0, 
    DOWN = 1, 
    LEFT = 2, 
    RIGHT = 3 
};

// Acción visible de un enemigo: el servidor la enciende y el cliente la anima
// (la muerte la deriva el cliente de la vida = 0).
enum class EnemyAction : uint8_t { Idle = 0, Attack = 1, Damage = 2 };

// Como acordamos, usamos HAND para levantar objetos
enum class ToolType : uint8_t { 
    HAND = 0,
    PICKAXE = 1, 
    SHOVEL = 2, 
    WATERING_CAN = 3, 
    SWORD = 4 
};

#endif // COMMON_CONSTANTS_H
