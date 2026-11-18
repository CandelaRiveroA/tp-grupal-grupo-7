#ifndef ENEMY_CONTROLLER_H
#define ENEMY_CONTROLLER_H

#include <cstdint>
#include <random>

#include "../entities/entity_controller.h"

// Reparte los enemigos de la sala: exactamente un Skull y, por cada punto de
// aparición, esqueletos, manadas de orcos o parejas de vampiros según la sala.
class EnemyController: public EntityController {
public:
    explicit EnemyController(uint8_t sala, uint32_t seed = std::random_device{}());

protected:
    void clear_generation(World& world) override;
    void spawn_at(World& world, uint16_t x, uint16_t y) override;

private:
    uint8_t sala;
    // Lo pone el primer spawn_at de cada generación; clear_generation lo reearma.
    bool skull_placed;
};

#endif  // ENEMY_CONTROLLER_H
