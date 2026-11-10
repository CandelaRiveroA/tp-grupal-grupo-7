#ifndef MINERAL_H
#define MINERAL_H

#include "../../../common/common_constants.h"
#include "../entities/i_entity.h"
#include "../inventory/item_types.h"

class Mineral: public IEntity {
private:
    uint8_t hits_remaining;
    uint8_t max_hits;

public:
    Mineral(uint16_t x, uint16_t y, uint8_t hits_remaining);
    virtual ~Mineral() = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    std::optional<CellSnapshot> snapshot() const override;

    // Item que deja al agotarse (identifica el tipo de mineral).
    virtual ItemID get_drop_id() const = 0;

    void update(World& w) override;

protected:
    virtual void mine_mineral(World& w) = 0;
    // En el futuro se puede agregar el jugador si es necesario para completar
    //  misiones o para un sistema de experiencia, pero por ahora no es necesario.
};

#endif  // MINERAL_H
