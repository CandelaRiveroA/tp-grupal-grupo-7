#ifndef SKULL_H
#define SKULL_H

#include <cstdint>

#include "enemy.h"

// Enemigo invulnerable y estacionario: tira llamas a distancia al jugador más
// cercano dentro de rango. No se puede matar: el jugador debe huir.
class Skull: public Enemy {
public:
    Skull(uint16_t x, uint16_t y);
    ~Skull() override = default;

    // Invulnerable: el golpe no le hace nada.
    bool take_damage(uint8_t incoming) override;

    // Nunca se llama: take_damage nunca devuelve true.
    void on_death(World& w) override;

protected:
    void behave(World& w) override;

private:
    uint8_t flame_tick;
};

#endif  // SKULL_H
