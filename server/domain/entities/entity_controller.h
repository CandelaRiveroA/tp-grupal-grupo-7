#ifndef ENTITY_CONTROLLER_H
#define ENTITY_CONTROLLER_H

#include <cstdint>
#include <memory>
#include <random>

class IEntity;
class World;

// Población que un controller repone en el mundo. El flujo vive acá
// (limpiar la generación vieja -> juntar celdas libres -> pedirle a la
// especialización qué poner); clear_generation() y create_entity() los
// define cada controller (hoy MineController, más adelante EnemyController).
class EntityController {
public:
    EntityController(uint8_t density_percent, uint32_t seed);
    virtual ~EntityController() = default;

    // Regenera: limpia lo que este controller puebla y coloca la nueva
    // generación en celdas libres, sin pisar la celda reservada (spawn)
    // ni las que ya tienen algo (cama, animales).
    void populate(World& world, uint16_t reserved_x, uint16_t reserved_y);

protected:
    // Qué entidades borra al regenerar (cada controller, lo suyo).
    virtual void clear_generation(World& world) = 0;
    // Qué entidad crea en una celda libre ya elegida.
    virtual std::unique_ptr<IEntity> create_entity(uint16_t x, uint16_t y) = 0;

    // Generador compartido: la base lo usa para barajar y la especialización
    // para sus propios sorteos (pesos, drops, etc.).
    std::mt19937 rng;

private:
    // % de las celdas libres que ocupa una generación.
    uint8_t density_percent;
};

#endif  // ENTITY_CONTROLLER_H
