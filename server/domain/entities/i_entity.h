#ifndef IENTITY_H
#define IENTITY_H

#include <cstdint>
#include <optional>

#include "../../../common/common_constants.h"
#include "../inventory/item_types.h"

class World;
class Player;

// Descripción neutral de cómo se ve una celda, para que el mundo la muestre
// sin conocer el tipo concreto de la entidad.
struct CellSnapshot {
    enum class Kind { Resource, Dropped };

    Kind kind;
    ItemID item;
    uint8_t hits_taken;
    uint8_t hits_to_destroy;
};

class IEntity {
protected:
    uint16_t pos_x;
    uint16_t pos_y;

public:
    IEntity(uint16_t x, uint16_t y);
    virtual ~IEntity() = default;

    uint16_t get_x() const;
    uint16_t get_y() const;

    void set_position(uint16_t x, uint16_t y);

    // Determina si otra entidad (como el jugador o enemigos) puede caminar SOBRE esta entidad.
    // Por defecto, asumimos que la mayoria bloquean el paso.
    // DroppedItem podría sobrescribir esto para retornar 'true' si quieres que el
    // jugador pueda pararse en la misma casilla que un ítem tirado antes de recogerlo.
    virtual bool is_walkable() const = 0;

    virtual void interact(Player& p, ToolType tool, World& w) = 0;

    // Nullopt si la entidad no se muestra como contenido de celda.
    virtual std::optional<CellSnapshot> snapshot() const;

    // Método para que las entidades se actualicen en cada frame del GameLoop.
    virtual void update();
};

#endif  // IENTITY_H
