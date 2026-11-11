#ifndef ENEMY_CONTROLLER_H
#define ENEMY_CONTROLLER_H

#include <cstdint>
#include <memory>
#include <random>

#include "../entities/entity_controller.h"

// Reparte los enemigos de la sala (un Skull por cuarto, ...).
// shortcut: solo firmas, sin .cpp ni include todavía; se implementa cuando
// estén todos los enemigos. Instanciarlo antes falla en el link.
class EnemyController: public EntityController {
public:
    explicit EnemyController(uint8_t sala, uint32_t seed = std::random_device{}());

protected:
    void clear_generation(World& world) override;
    std::unique_ptr<IEntity> create_entity(uint16_t x, uint16_t y) override;

private:
    uint8_t sala;
};

#endif  // ENEMY_CONTROLLER_H
