#ifndef FLAME_H
#define FLAME_H

#include <cstdint>
#include <string>

#include "../../../common/common_constants.h"

// Llama de un skull: no vive en el grid, así que no bloquea celdas por
// construcción. La mueve World::tick_flames, que la quita al pegar, al chocar
// contra una pared o al llegar a la celda objetivo (la que ocupaba el jugador
// al lanzar). direction se recalcula en cada paso: vuela hacia el objetivo.
struct Flame {
    uint16_t id;
    std::string kind;
    uint16_t x;
    uint16_t y;
    Direction direction;
    uint8_t damage;
    uint16_t target_x;
    uint16_t target_y;
};

#endif  // FLAME_H
