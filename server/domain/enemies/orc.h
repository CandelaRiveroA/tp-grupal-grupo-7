#ifndef ORC_H
#define ORC_H

#include "enemy.h"

// Enemigo de dificultad media: cuerpo a cuerpo, spawnea en manadas de 3 a 6
// y al morir tiene una probabilidad extra de soltar plata (además del oro
// común a todos los enemigos). No revive.
class Orc: public Enemy {
public:
    Orc(uint16_t x, uint16_t y);
    // Probabilidad de plata distinta de la de fábrica (para tests).
    Orc(uint16_t x, uint16_t y, uint8_t silver_drop_percent);
    ~Orc() override = default;

    // Al morir suelta plata con la probabilidad de la especie.
    void on_death(World& w) override;
    std::string kind_name() const override;

private:
    uint8_t silver_drop_percent;
};

#endif  // ORC_H
