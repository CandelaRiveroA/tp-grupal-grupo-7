#ifndef LAPIS_ROCK_H
#define LAPIS_ROCK_H

#include "mineral.h"

class LapisRock: public Mineral {
public:
    LapisRock(uint16_t x, uint16_t y);
    ~LapisRock() override = default;
    ItemID get_drop_id() const override;

protected:
    void mine_mineral(World& w) override;
};

#endif  // LAPIS_ROCK_H
