#ifndef STRAWBERRY_H
#define STRAWBERRY_H

#include "crop.h"

class Strawberry : public Crop {
public:
    // 4 días crecimiento, 2 regadas por día, soltar STRAWBERRY, precio 25[cite: 2]
    Strawberry(uint16_t x, uint16_t y);
    ~Strawberry() override = default;
    
    void harvest(World& w) override;
};

#endif // STRAWBERRY_H
