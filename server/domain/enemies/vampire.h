#ifndef VAMPIRE_H
#define VAMPIRE_H

#include "enemy.h"

// Enemigo de dificultad alta: cuerpo a cuerpo, spawnea en pareja y su
// particularidad es que revive mientras quede sangre en el mapa (la dejan los
// golpes que dejan al jugador por debajo del 20 % de vida). Cuando muere uno
// de la pareja, el otro entra en frenesí y duplica su habilidad especial.
class Vampire: public Enemy {
public:
    Vampire(uint16_t x, uint16_t y);
    ~Vampire() override;

    // Enlaza a esta pareja (el enlace mutuo lo hace cada lado).
    void set_partner(Vampire* other);

    // Al morir: consume un charco de sangre para revivir y enfurece a la pareja.
    void on_death(World& w) override;
    std::string kind_name() const override;

    // Daño del próximo golpe (x2 con frenesí de ataque).
    uint8_t get_damage() const override;

    // Persigue varios pasos por tick (speed * speed_frenzy).
    bool pursue(World& w) override;

    // Frenesí: duplica la habilidad especial de la especie (daño o velocidad).
    virtual void enter_frenzy();

protected:
    // Ctor para la variante (la vampira): cada especie fija su balance.
    Vampire(uint16_t x, uint16_t y, uint8_t health, uint8_t damage,
            uint8_t gold_drop_percent, uint8_t speed);

    // Acciones por tick = speed * speed_frenzy (el frenesí duplica uno de los dos).
    uint8_t speed;
    uint8_t attack_frenzy;
    uint8_t speed_frenzy;

private:
    // Puntero no-owning a la pareja. ~Vampire() lo desenlaza en ambas puntas
    // para que la pareja que sobrevive no quede con un puntero colgado.
    Vampire* partner;
};

#endif  // VAMPIRE_H
