#ifndef MINE_CONTROLLER_H
#define MINE_CONTROLLER_H

#include <memory>

#include "../entities/entity_controller.h"

// Genera los minerales de la caverna según los rates de aparición.
// Se vuelve a llamar populate() al cambiar de día: la sala se repone con
// una generación nueva (semilla aleatoria por construcción del controller).
class MineController: public EntityController {
public:
    // `sala` = sala actual de la caverna; el oro recién aparece desde la 5ta.
    explicit MineController(uint8_t sala, uint32_t seed = std::random_device{}());

protected:
    void clear_generation(World& world) override;
    std::unique_ptr<IEntity> create_entity(uint16_t x, uint16_t y) override;

private:
    uint8_t sala;
};

#endif  // MINE_CONTROLLER_H
