#ifndef IENTITY_H
#define IENTITY_H

#include <cstdint>
#include "../../../common/common_constants.h"

class World;
class Player;

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
    
    // Método para que las entidades se actualicen en cada frame del GameLoop.
    virtual void update();
};

#endif // IENTITY_H
