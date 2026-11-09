#ifndef PUMPKIN_H
#define PUMPKIN_H

#include "crop.h"

class Pumpkin : public Crop {
public:
    Pumpkin(uint16_t x, uint16_t y);
    ~Pumpkin() override = default;
    
    void harvest(World& w) override;
};

#endif // PUMPKIN_H
