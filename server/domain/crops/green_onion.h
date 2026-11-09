#ifndef GREEN_ONION_H
#define GREEN_ONION_H

#include "crop.h"

class GreenOnion : public Crop {
public:
    GreenOnion(uint16_t x, uint16_t y);
    ~GreenOnion() override = default;
    
    void harvest(World& w) override;
};

#endif // GREEN_ONION_H
