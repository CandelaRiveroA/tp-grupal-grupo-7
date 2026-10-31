#ifndef RESOURCE_HIT_H
#define RESOURCE_HIT_H

#include <cstdint>

// Evento de dominio: un golpe válido, incluido el que agota el recurso.
struct ResourceHit {
    uint16_t x;
    uint16_t y;
    bool destroyed;
};

#endif  // RESOURCE_HIT_H
