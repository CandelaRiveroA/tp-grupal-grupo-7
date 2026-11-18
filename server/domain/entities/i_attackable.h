#ifndef IATTACKABLE_H
#define IATTACKABLE_H

#include <cstdint>

// Capacidades de combate: una entidad que puede recibir golpes.
class IAttackable {
public:
    virtual ~IAttackable() = default;

    // Aplica `incoming` y devuelve true si con ese golpe quedó sin vida.
    virtual bool take_damage(uint8_t incoming) = 0;
};

#endif  // IATTACKABLE_H
