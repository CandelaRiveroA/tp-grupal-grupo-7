#ifndef SKULL_H
#define SKULL_H

#include <cstdint>

#include "enemy.h"

// Enemigo invulnerable que tira llamas a distancia al jugador más cercano
// dentro de rango. No se puede matar: el jugador debe huir. Se acerca hasta
// kShotDistance para que la línea de tiro casi nunca choque con una pared.
class Skull: public Enemy {
public:
    Skull(uint16_t x, uint16_t y);
    ~Skull() override = default;

    // Invulnerable: el golpe no le hace nada.
    bool take_damage(uint8_t incoming) override;

    // Nunca se llama: take_damage nunca devuelve true.
    void on_death(World& w) override;
    std::string kind_name() const override;

    // Un paso de persecución: limpia la acción para rearmar su animación y
    // se acerca al jugador (sin pegarle) hasta kShotDistance. Las llamas las
    // lanza el reloj lento en behave().
    bool pursue(World& w) override;

protected:
    void behave(World& w) override;

private:
    uint8_t flame_tick;
};

#endif  // SKULL_H
