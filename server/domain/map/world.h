#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../entities/player.h"

#include "cell.h"
#include "flame.h"
#include "player_pickup.h"
#include "resource_hit.h"

class Animal;
class Enemy;
class Crop;

struct GiftOffer {
    uint8_t sender_id;
    uint8_t receiver_id;
    ItemID item;
};

class World {
private:
    uint16_t width;
    uint16_t height;

    std::vector<std::vector<Cell>> grid;
    // No poseen: los dueños son las celdas. Se mantienen para tick_day y ordeñe.
    std::vector<Animal*> animals;
    // No poseen: los dueños son las celdas. Se mantienen para tick_enemies;
    // remove_entity también los saca de acá.
    std::vector<Enemy*> enemies;
    // Próximo id de enemigo (lo ve el cliente para seguirlo entre mensajes).
    uint16_t next_enemy_id;
    // Llamas en vuelo (poseídas por el mundo, no por las celdas: no bloquean).
    std::vector<Flame> flames;
    // Próximo id de proyectil (rango distinto al de enemigos y animales:
    // ViewMotion empareja las entidades por id en una sola lista).
    uint16_t next_projectile_id;
    std::vector<Crop*> crops;
    std::vector<ResourceHit> resource_hits;
    std::vector<PlayerPickup> pickups;
    // Charcos de sangre (globales para todo el mapa): los deja damage_player
    // cuando un jugador queda por debajo del 20 % de vida y los consumen los
    // vampiros al morir. La posición queda para un futuro render.
    std::vector<std::pair<uint16_t, uint16_t>> blood_puddles;

    // Sueño: jugadores en la cama (sleepers) y los que ya dieron OK
    // (accepted). accepted ⊆ sleepers, así que vaciar sleepers vacía
    // accepted. Quien está en sleepers no se puede mover.
    std::unordered_set<uint8_t> sleepers;
    std::unordered_set<uint8_t> accepted;
    // Muertos esperando la animación de desmayo antes de ir a la cama:
    // id -> pasos de EnemyTick restantes. Match los avanza con
    // tick_death_to_bed.
    std::unordered_map<uint8_t, uint8_t> death_to_bed;
    // Día terminado y esperando que todos despierten (woke ⊆ players). Mientras
    // tanto el reloj y los animales están quietos.
    bool waking;
    std::unordered_set<uint8_t> woke;
    // Transiciones del día, consumidas por Match (una sola vez cada una).
    bool day_ended;
    bool day_started;
    // Día de juego: de 1 a 28, después del 28 vuelve al 1.
    uint8_t day;
    // Para manejo de regalos
    std::vector<GiftOffer> pending_gifts;

    // TODO: Persistencia del inventario del jugador.

    // Diccionario de jugadores activos en este mapa (por ID de cliente/jugador)
    std::unordered_map<uint8_t, std::unique_ptr<Player>> players;
    void apply_consume_energy(uint8_t player_id, float amount);
public:
    World(uint16_t w, uint16_t h);
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // --- Detección de Colisiones en O(1) ---
    bool can_walk_to(uint16_t x, uint16_t y) const;

    // --- Gestión de Jugadores ---
    void add_player(uint8_t player_id, std::unique_ptr<Player> player);
    void remove_player(uint8_t player_id);
    void move_player(uint8_t player_id, Direction dir);
    const Player& get_player(uint8_t player_id) const;
    Player& get_player(uint8_t player_id);
    std::vector<uint8_t> get_player_ids() const;

    uint16_t get_width() const;
    uint16_t get_height() const;
    const std::vector<IEntity*> get_entity_at(uint16_t x, uint16_t y) const;

    // --- Gestión de Entidades Físicas ---
    void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity);
    void remove_entity(IEntity* target_entity);
    void remove_entity_at(uint16_t x, uint16_t y);

    void add_animal(std::unique_ptr<Animal> animal);
    void add_enemy(std::unique_ptr<Enemy> enemy);

    // Manada de 3 a 6 orcos en celdas caminables con radio <= 2 de (x, y).
    void spawn_orc_pack(uint16_t x, uint16_t y);

    // Pareja enlazada de vampiros (1 vampiro + 1 vampira) en celdas
    // caminables con radio <= 1 de (x, y).
    void spawn_vampire_pair(uint16_t x, uint16_t y);

    // Avanza un día: cada animal reacciona a su update (huevos, leche lista).
    void tick_day();
    // Pasa al día siguiente (contador de día) y corre tick_day. Lo llaman el
    // timeout y el sueño.
    void advance_day();
    uint8_t get_day() const;

    // Un paso del reloj de animales: cada uno se mueve en su dirección actual
    // o elige una nueva si expiró el tiempo.
    void tick_animals();

    // Borra todos los enemigos del mapa (vivos y cadáveres), sin dejar
    // punteros colgantes en la lista de ticks. Lo usa EnemyController.
    void clear_enemies();

    // Borra las llamas en vuelo. Lo usa EnemyController al regenerar.
    void clear_flames();

    // Un paso del reloj lento de enemigos (4 s): revive a los muertos, limpia
    // la acción, deambula (si nadie está en rango) y el skull lanza llamas.
    void tick_enemies();

    // Un paso del reloj rápido de enemigos (400 ms, el paso del jugador):
    // cada enemigo vivo da un paso de persecución y pega con cooldown.
    // Devuelve true si cambió algo (posición o acción) y hay que sincronizar.
    bool tick_enemies_pursuit();

    // Un paso de vuelo de las llamas: daña al contacto, muere contra una
    // pared o al llegar a su celda objetivo. Devuelve true si había llamas.
    bool tick_flames();

    // Lanza una llama desde (x, y) hacia la celda objetivo; la dirección
    // inicial la elige el mundo hacia el objetivo.
    void spawn_flame(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y,
                     uint8_t damage, const std::string& kind);

    // Llamas en vuelo, para que world_view las emita sin conocer Flame.
    std::vector<ProjectileSnapshot> projectile_snapshots() const;

    // Charcos de sangre vivos del día (celda de cada uno): es estado
    // persistente (se seca al terminar el día o lo consume una vampira).
    // Seam de lectura para que WorldView los transmita a todos.
    const std::vector<std::pair<uint16_t, uint16_t>>& blood_puddle_list() const;

    // vacia los charcos de sangre restantes al finalizar el día.
    void clear_blood_puddle();

    // Golpe de un enemigo al jugador. Devuelve true si quedó sin vida.
    bool damage_player(uint8_t player_id, uint8_t damage);

    // Consume el último charco de sangre: true = quedaba sangre (la usa el
    // vampiro que murió para programar su revive).
    bool consume_blood_puddle();

    // Hay al menos un charco de sangre en el mapa (lectura sin consumir):
    // la usa el vampiro al morir para decidir si programa el revive.
    bool has_blood_puddle() const;

    // Avanza la espera de los muertos hacia la cama; al vencer, los
    // teletransporta y registra su sueño (el resto del flujo es el de
    // siempre). Devuelve true si alguien llegó en este paso, para que
    // Match sincronice aunque no haya enemigos ni llamas en vuelo.
    bool tick_death_to_bed();

    // Mueve una entidad a otra celda; no hace nada si el destino está bloqueado.
    void move_entity(IEntity* entity, uint16_t x, uint16_t y);

    // ---
    void use_tool(uint8_t player_id);
    void record_resource_hit(uint16_t x, uint16_t y, bool destroyed);
    std::vector<ResourceHit> take_resource_hits();
    void record_pickup(uint8_t player_id, ItemID item);
    std::vector<PlayerPickup> take_pickups();
    void set_player_facing(uint8_t player_id, Direction dir);
    void select_slot(uint8_t player_id, uint8_t slot);
    // Personalizacion del personaje del jugador (indices de capa, sin validar).
    void set_player_appearance(uint8_t player_id, const CharacterAppearance& appearance);
    bool try_plant_seed(uint8_t player_id, uint16_t tx, uint16_t ty);
    // TODO: Ver si está de más, ya que el jugador puede interactuar con la celda de frente, no con la propia. AUTO-PICK-UP
    void take_item(uint8_t player_id);

    // -- - Actualización de Entidades ---
    void dry_soil();
    void process_end_of_day_floor();

    // Para tests
    Cell& get_cell(uint16_t x, uint16_t y);
    const Cell& get_cell(uint16_t x, uint16_t y) const;
    
    // --- Sueño (votación para dormir y pasar de día) ---
    // El jugador pisa la cama: entra en la votación y queda sin movimiento.
    void record_sleep_request(uint8_t player_id);
    // Responde al aviso: accept = confirma dormir, false = cancelar (sale
    // de la cama, le devuelve el movimiento y borra solo su OK).
    void answer_sleep(uint8_t player_id, bool accept);
    uint8_t sleeping_count() const;
    bool is_sleeping(uint8_t player_id) const;
    // Cancela la votación abierta sin mover a nadie.
    void clear_sleep();
    void set_player_position(uint8_t player_id, uint16_t x, uint16_t y);
    void kill_player(uint8_t player_id);
    void faint_player(uint8_t player_id);
    void reset_player_for_day(uint8_t player_id);
    void mark_all_fainted_by_timeout();

    // --- Fin de día: todos tienen que despertar (OK) antes de arrancar el siguiente ---
    // Termina el día (timeout o sueño) y congela el reloj hasta despertar.
    void end_day();
    // El jugador apretó OK: cuando despiertan todos, el día avanza.
    void answer_wake(uint8_t player_id);
    bool is_waking() const;
    // true una sola vez por transición. Los consume Match.
    bool take_day_ended();
    bool take_day_started();

    //funcion para avisar del regalo de un item a otro jugador
    void notify_gift_request(uint8_t sender_id, uint8_t receiver_id, ItemID item_id);
    void process_gift_acceptance(uint8_t sender_id, uint8_t receiver_id);
    std::vector<GiftOffer> take_gift_requests();

private:
    void try_advance_sleep();
    void try_start_day();
    // Daña al jugador vivo que esté parado en (x, y); true si pegó (y la
    // llama se consume).
    bool damage_players_at(uint16_t x, uint16_t y, uint8_t damage);
};

#endif  // WORLD_H
