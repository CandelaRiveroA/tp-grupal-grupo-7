#ifndef PLAYER_PICKUP_H
#define PLAYER_PICKUP_H

#include <cstdint>

#include "../inventory/item_types.h"

// Evento de dominio: un jugador recogió un objeto del mundo. Nace en el
// momento del pickup, no de comparar inventarios.
struct PlayerPickup {
    uint8_t player_id;
    ItemID item;
};

#endif  // PLAYER_PICKUP_H
