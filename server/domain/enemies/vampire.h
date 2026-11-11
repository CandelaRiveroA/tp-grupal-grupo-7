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
    ~Vampire() override = default;

    // Enlaza a esta pareja (el enlace mutuo lo hace cada lado).
    void set_partner(Vampire* other);

    // Al morir: consume un charco de sangre para revivir y enfurece a la pareja.
    void on_death(World& w) override;

    // Daño del próximo golpe (x2 con frenesí de ataque).
    uint8_t get_damage() const override;

    // Frenesí: duplica la habilidad especial de la especie (daño o velocidad).
    virtual void enter_frenzy();

protected:
    // Ctor para la variante (la vampira): cada especie fija su balance.
    Vampire(uint16_t x, uint16_t y, uint8_t health, uint8_t damage,
            uint8_t gold_drop_percent, uint8_t speed);

    void behave(World& w) override;

    // Acciones por tick = speed * speed_frenzy (el frenesí duplica uno de los dos).
    uint8_t speed;
    uint8_t attack_frenzy;
    uint8_t speed_frenzy;

private:
    // shortcut: puntero no-owning a la pareja; queda colgado si la pareja sale
    // del mundo (mismo patrón que World::enemies) y hoy nada retira enemigos:
    // cuando haya retiro hay que limpiarlo acá.
    Vampire* partner;
};

#endif  // VAMPIRE_H
