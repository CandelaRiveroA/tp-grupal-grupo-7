#ifndef POTATO_H
#define POTATO_H

#include "crop.h"

class Potato : public Crop {
public:
    Potato(uint16_t x, uint16_t y);
    ~Potato() override = default;
    
    void harvest(World& w) override;
};

#endif // POTATO_H
