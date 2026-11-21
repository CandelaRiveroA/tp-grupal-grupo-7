#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>
#include <optional>
#include <string>
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
    // Identificador que asigna el mundo al registrarlo (lo ve el cliente).
    uint16_t id;
    // Hacia dónde mira: la última dirección intentada al moverse.
    Direction facing;
    // Acción que el cliente anima (golpe o daño); la enciende el servidor.
    EnemyAction action;
    // Ticks de persecución que faltan para poder volver a pegar (cooldown).
    uint8_t attack_cooldown;
    // Rumbo y tiempos del deambular (patrón Animal): solo fuera de combate.
    Direction wander_dir;
    uint8_t ticks_to_turn;
    uint8_t ticks_to_pause;

public:
    Enemy(uint16_t x, uint16_t y, uint8_t health, uint8_t damage, uint8_t gold_drop_percent);
    ~Enemy() override = default;
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;
    Enemy(Enemy&&) = delete;
    Enemy& operator=(Enemy&&) = delete;

    uint8_t get_health() const;
    void set_id(uint16_t new_id);

    // Nombre de la especie en enemies.toml (vocabulario compartido con el cliente).
    virtual std::string kind_name() const = 0;
    std::optional<EnemySnapshot> enemy_snapshot() const override;

    // Daño del próximo golpe: las especies con frenesí lo multiplican.
    virtual uint8_t get_damage() const;

    // IAttackable: false si ya estaba muerto; true = murió con este golpe.
    bool take_damage(uint8_t incoming) override;

    // Un paso del reloj rápido de persecución (400 ms): limpia la acción para
    // rearmar el flanco de animación del cliente, baja el cooldown y da un
    // paso hacia el jugador. Devuelve true si cambió algo (posición o acción).
    virtual bool pursue(World& w);

    // IMovable
    bool move(World& w, Direction dir) override;

    // Consecuencia de la muerte en el mundo (revive, drop, retiro). La llama
    // quien ataca, porque take_damage no recibe World.
    virtual void on_death(World& w) = 0;

    // Consumo necesario para revivir: por defecto libre; el vampiro consume
    // un charco de sangre.
    virtual bool try_consume_for_revive(World& /*w*/) { return true; }

    // Contrato IEntity
    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    void update(World& w) override;

protected:
    // Un paso de IA de la especie en el reloj lento (4 s): update() lo llama
    // mientras esté con vida. Por defecto deambula; el skull lo sobreescribe
    // para lanzar llamas.
    virtual void behave(World& w);

    // `steps` pasos de persecución con el mismo cooldown de ataque (el
    // vampiro da varios por tick). Devuelve true si cambió algo.
    bool pursue_steps(World& w, uint8_t steps);

    // Deambula un paso (mismo patrón que Animal): se salta si hay un jugador
    // en rango de aggro, porque a ese lo persigue el reloj rápido.
    void wander(World& w);

    // Jugador más cercano dentro del radio dado, o nullptr.
    const Player* find_nearest_player(World& w, int range) const;

    // Persecución cuerpo a cuerpo compartida: persigue al jugador vivo más
    // cercano dentro de kAggroRange y le pega si está en las 8 vecinas y el
    // cooldown lo permite. Devuelve true si pegó o se movió.
    bool chase_player(World& w);

    // Un paso hacia `target`: primero el eje con más distancia y, si está
    // bloqueado, el otro. Sin pegarle (lo decide el llamador). Devuelve
    // true si se movió.
    bool step_toward(World& w, const Player& target);

    // Suelta un orbe de `item` en la celda con la probabilidad dada.
    void drop_orb(World& w, ItemID item, uint16_t price, uint8_t percent);
};

#endif  // ENEMY_H
