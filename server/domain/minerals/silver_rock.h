#ifndef SILVER_ROCK_H
#define SILVER_ROCK_H

#include "mineral.h"

class SilverRock: public Mineral {
public:
    SilverRock(uint16_t x, uint16_t y);
    ~SilverRock() override = default;
    ItemID get_drop_id() const override;

protected:
    void mine_mineral(World& w) override;
};

#endif  // SILVER_ROCK_H
