#ifndef MINERAL_H
#define MINERAL_H

#include "../entities/i_entity.h"
#include "../../../common/common_constants.h"

class Mineral : public IEntity {
private:
    uint8_t hits_remaining;
public:
    Mineral(uint16_t x, uint16_t y, uint8_t hits_remaining) : IEntity(x, y), hits_remaining(hits_remaining) {};
    virtual ~Mineral() = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;

protected:
    virtual void mine_mineral(Player& p, World& w) = 0; 
};

#endif // MINERAL_H
