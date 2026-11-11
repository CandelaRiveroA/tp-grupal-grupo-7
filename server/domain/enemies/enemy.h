#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>
#include <utility>  // cpplint: lo pide por la declaración de move

#include "../entities/i_attackable.h"
#include "../entities/i_entity.h"
#include "../entities/i_movable.h"

class World;

// Enemigo abstracto: entidad con combate cuerpo a cuerpo. Comparte con todas
// las especies el ciclo de vida (vida, daño, muerte y revive programado); el
// comportamiento propio de cada una vive en behave() y su consecuencia en el
// mundo en on_death().
class Enemy: public IEntity, public IMovable, public IAttackable {
protected:
    uint8_t health;
    uint8_t max_health;
    uint8_t damage;
    // Ticks que faltan para revivir con vida llena. Lo pone on_death al
    // morir; 0 = el enemigo no revive.
    uint8_t ticks_to_revive;
    // Probabilidad (0 a 100) de soltar un orbe de oro al morir.
    uint8_t gold_drop_percent;

public:
    Enemy(uint16_t x, uint16_t y, uint8_t health, uint8_t damage, uint8_t gold_drop_percent);
    ~Enemy() override = default;
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;
    Enemy(Enemy&&) = default;
    Enemy& operator=(Enemy&&) = default;

    uint8_t get_health() const;

    // IAttackable: false si ya estaba muerto; true = murió con este golpe.
    bool take_damage(uint8_t incoming) override;

    // IMovable
    bool move(World& w, Direction dir) override;

    // Consecuencia de la muerte en el mundo (revive, drop, retiro). La llama
    // quien ataca, porque take_damage no recibe World.
    virtual void on_death(World& w) = 0;

    // Contrato IEntity
    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    void update(World& w) override;

protected:
    // Un paso de IA de la especie (perseguir, atacar, ...). update() lo
    // llama mientras esté con vida.
    virtual void behave(World& w) = 0;

    // Persecución cuerpo a cuerpo compartida: persigue al jugador vivo más
    // cercano dentro de kAggroRange y le pega si está en las 8 vecinas.
    void chase_player(World& w);

    // Suelta un orbe de `item` en la celda con la probabilidad dada.
    void drop_orb(World& w, ItemID item, uint16_t price, uint8_t percent);
};

#endif  // ENEMY_H
