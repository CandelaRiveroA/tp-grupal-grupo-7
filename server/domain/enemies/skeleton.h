#ifndef SKELETON_H
#define SKELETON_H

#include "enemy.h"

// Primer enemigo del juego: cuerpo a cuerpo, dificultad baja, spawnea solo y
// su particularidad es que revive pasado un tiempo en la misma celda.
class Skeleton: public Enemy {
public:
    Skeleton(uint16_t x, uint16_t y);
    ~Skeleton() override = default;

    // Al morir programa el revive; el mundo no se toca (sigue en la celda).
    void on_death(World& w) override;
    std::string kind_name() const override;
};

#endif  // SKELETON_H
