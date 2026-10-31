#ifndef COAL_ROCK_H
#define COAL_ROCK_H

#include "mineral.h"

class CoalRock: public Mineral {
public:
    CoalRock(uint16_t x, uint16_t y);
    ~CoalRock() override = default;
    ItemID get_drop_id() const override;

protected:
    void mine_mineral(World& w) override;
};

#endif  // COAL_ROCK_H
