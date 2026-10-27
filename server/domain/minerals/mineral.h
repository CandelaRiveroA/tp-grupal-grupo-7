#ifndef MINERAL_H
#define MINERAL_H

#include "../entities/i_entity.h"
#include "../../../common/common_constants.h"

class Mineral : public IEntity {
private:
    uint8_t hits_remaining;
public:
    Mineral(uint16_t x, uint16_t y, uint8_t hits_remaining);
    virtual ~Mineral() = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;

protected:
    virtual void mine_mineral(World& w) = 0; 
    //En el futuro se puede agregar el jugador si es necesario para completar
    // misiones o para un sistema de experiencia, pero por ahora no es necesario.
};

#endif // MINERAL_H
