#ifndef ANIMAL_H
#define ANIMAL_H

#include "../entities/i_entity.h"
#include <cstdint>

class World;

class Animal : public IEntity {
protected:
    uint8_t days_passed;
public:
    Animal(uint16_t x, uint16_t y);
    virtual ~Animal() = default;
    Animal(const Animal&) = delete;
    Animal& operator=(const Animal&) = delete;
    Animal(Animal&&) = default;
    Animal& operator=(Animal&&) = default;

    virtual void produce(World& w) = 0;
    
    // --- Lógica de movimiento comentada hasta implementar el GameLoop ---
    // void move(int dx, int dy) override;
    // void wander(World& w);
};

#endif // ANIMAL_H