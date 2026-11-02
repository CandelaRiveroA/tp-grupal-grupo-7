#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <optional>
#include <string>

#include "../entities/i_entity.h"

class World;

class Animal: public IEntity {
protected:
    uint8_t days_passed;
    uint8_t id;

public:
    Animal(uint16_t x, uint16_t y);
    virtual ~Animal() = default;
    Animal(const Animal&) = delete;
    Animal& operator=(const Animal&) = delete;
    Animal(Animal&&) = default;
    Animal& operator=(Animal&&) = default;

    uint8_t get_id() const;

    // Vocabulario compartido con el cliente (characters.toml).
    virtual std::string kind_name() const = 0;

    virtual void produce(World& w) = 0;

    std::optional<AnimalSnapshot> animal_snapshot() const override;
};

#endif  // ANIMAL_H
