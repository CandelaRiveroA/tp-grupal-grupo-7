#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../animals/animal_purchase.h"
#include "../community/community_center.h"
#include "world_model/World.h"
#include "../community/community_set_catalog.h"
#include "../community/community_set_rewards.h"
#include "../entities/player.h"

#include "dungeon_floor.h"
#include "farm.h"
#include "flame.h"
#include "player_pickup.h"
#include "resource_hit.h"
#include "scene.h"
#include "town.h"

class Animal;
class Barn;
class Enemy;
class Crop;
class Merchant;

struct GiftOffer {
    uint8_t sender_id;
    uint8_t receiver_id;
    ItemID item;
};

class World {
private:
    // Dueño de todos los escenarios de la partida (FARMHOUSE, FARM, TOWN, DUNGEON_1..10).
    std::map<SceneId, std::unique_ptr<Scene>> scenes;
    Farm* farm;
    Barn* barn;
    SceneId barn_scene_id_;
    Town* town;
    std::string spawn_map_;
    uint16_t spawn_x_ = 0;
    uint16_t spawn_y_ = 0;
    std::vector<std::pair<std::string, std::pair<uint16_t, uint16_t>>> cabin_beds_;
    std::map<int, DungeonFloor*> dungeon_floors;
    // Escena sobre la que se está ejecutando el tick o interacción actual
    // (permite que IEntity::interact / update sigan llamando a World& sin cambiar firmas).
    SceneId active_scene;

    // Último comerciante seleccionado por cada jugador al interactuar.
    std::unordered_map<uint8_t, Merchant*> selected_merchants;

    // Contadores globales de IDs (únicos en toda la partida).
    uint16_t next_enemy_id;
    uint16_t next_projectile_id;
    uint16_t next_orc_group_id;
    uint16_t next_vampire_pair_id;

    // Eventos globales consumidos por Match.
    std::vector<ResourceHit> resource_hits;
    std::vector<PlayerPickup> pickups;
    std::vector<uint8_t> gift_answers;
    std::vector<uint8_t> accepted_gifts;
    std::map<std::pair<uint8_t, uint8_t>, ItemID> offered_gifts;
    std::vector<GiftOffer> pending_gifts;

    // Sueño y fin de día.
    std::unordered_set<uint8_t> sleepers;
    std::unordered_set<uint8_t> accepted;
    std::unordered_map<uint8_t, uint8_t> death_to_bed;
    bool waking;
    std::unordered_set<uint8_t> woke;
    bool day_ended;
    bool day_started;
    uint8_t day;

    // Cheat de tiempo: valor deseado del reloj y reinicio pendiente del
    // DayClock (con su base de minutos).
    uint16_t desired_clock_minutes_ = 0;
    bool clock_minutes_dirty_ = false;
    std::optional<uint16_t> pending_clock_restart_;

    // Centro Comunitario (estado global cooperativo).
    CommunityCenter community_center;
    bool community_victory;
    uint8_t current_room;
    bool market_discount_active;
    bool double_sell_active;
    bool skull_survived_without_damage;
    CommunitySetCatalog community_set_catalog;
    CommunitySetRewards community_set_rewards;

    // Diccionario global de jugadores activos en la partida.
    std::unordered_map<uint8_t, std::unique_ptr<Player>> players;

    void apply_consume_energy(uint8_t player_id, uint16_t amount);
    CommunitySet community_set_from_name(const std::string& set_name) const;
    void apply_set_reward(CommunitySet set);
    void try_advance_sleep();
    void try_start_day();
    DungeonFloor* active_dungeon_floor();
    const DungeonFloor* active_dungeon_floor() const;

public:
    World(uint16_t w, uint16_t h);
    World(const worldmodel::World& wm);
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // --- Gestión de Escenarios ---
    Scene& get_scene(SceneId id);
    const Scene& get_scene(SceneId id) const;
    const Scene& get_scene_of(uint8_t player_id) const;
    std::vector<SceneId> get_scene_ids() const;
    void set_active_scene(SceneId id);
    SceneId get_active_scene() const;
    Farm& get_farm();
    Town& get_town();
    const std::string& spawn_map() const;
    uint16_t spawn_x() const;
    uint16_t spawn_y() const;
    std::size_t cabin_count() const;
    std::pair<std::string, std::pair<uint16_t, uint16_t>> cabin_bed(std::size_t k) const;
    DungeonFloor& get_dungeon_floor(uint8_t room_number);
    void transfer_player(uint8_t player_id, SceneId target_scene, uint16_t x, uint16_t y);

    // --- Fachada espacial sobre active_scene (para entidades y compatibilidad) ---
    bool can_walk_to(uint16_t x, uint16_t y) const;
    uint16_t get_width() const;
    uint16_t get_height() const;
    SoilState get_soil_state(uint16_t x, uint16_t y) const;
    const std::vector<IEntity*> get_entity_at(uint16_t x, uint16_t y) const;
    void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity);
    void remove_entity(IEntity* target_entity);
    void remove_entity_at(uint16_t x, uint16_t y);
    void move_entity(IEntity* entity, uint16_t x, uint16_t y);

    // --- Gestión de Jugadores ---
    void add_player(uint8_t player_id, std::unique_ptr<Player> player);
    void remove_player(uint8_t player_id);
    void move_player(uint8_t player_id, Direction dir);
    void toggle_player_walking(uint8_t player_id);
    const Player& get_player(uint8_t player_id) const;
    Player& get_player(uint8_t player_id);
    std::vector<uint8_t> get_player_ids() const;
    std::vector<uint8_t> get_player_ids_in_scene(SceneId scene_id) const;
    void set_player_position(uint8_t player_id, uint16_t x, uint16_t y);
    void set_player_facing(uint8_t player_id, Direction dir);
    void select_slot(uint8_t player_id, uint8_t slot);
    void set_player_appearance(uint8_t player_id, const CharacterAppearance& appearance);

    // --- Animales (delegados al establo) ---
    void add_animal(std::unique_ptr<Animal> animal);
    void tick_animals();
    AnimalPurchaseResult buy_animals(Player& player, ItemID type, uint16_t quantity,
                                     uint16_t cost);
    void reward_animals(Player& player, ItemID type, uint16_t quantity);

    // --- Comerciantes (delegados a Town) ---
    void place_merchant(std::unique_ptr<Merchant> merchant);
    Merchant* get_nearest_merchant(uint8_t player_id) const;

    Merchant* get_merchant_for_command(uint8_t player_id);
    std::string process_merchant_command(uint8_t player_id, const std::string& text);

    // --- Enemigos, llamas y sangre (delegados a DungeonFloor) ---
    void add_enemy(std::unique_ptr<Enemy> enemy);

    // Manada de 3 a 6 orcos en celdas caminables con radio <= 2 de (x, y).
    void spawn_orc_pack(uint16_t x, uint16_t y);
    void spawn_vampire_pair(uint16_t x, uint16_t y);
    void clear_enemies();
    void clear_flames();
    void tick_enemies();
    bool tick_enemies_pursuit();
    bool tick_flames();
    void spawn_flame(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y,
                     uint8_t damage, const std::string& kind);
    std::vector<ProjectileSnapshot> projectile_snapshots() const;
    const std::vector<std::pair<uint16_t, uint16_t>>& blood_puddle_list() const;
    void clear_blood_puddle();
    bool consume_blood_puddle();
    bool has_blood_puddle() const;

    // --- Combate y daño ---
    bool damage_player(uint8_t player_id, uint8_t damage);
    bool damage_players_at(SceneId scene_id, uint16_t x, uint16_t y, uint8_t damage);
    void kill_player(uint8_t player_id);
    void faint_player(uint8_t player_id);
    bool tick_death_to_bed();
    void reset_player_for_day(uint8_t player_id);
    void mark_all_fainted_by_timeout();

    // --- Acciones del jugador ---
    void use_tool(uint8_t player_id);
    bool try_plant_seed(uint8_t player_id, uint16_t tx, uint16_t ty);
    void take_item(uint8_t player_id);
    void record_resource_hit(uint16_t x, uint16_t y, bool destroyed);
    std::vector<ResourceHit> take_resource_hits();
    void record_pickup(uint8_t player_id, ItemID item);
    std::vector<PlayerPickup> take_pickups();

    // --- Día y Sueño ---
    void tick_day();
    void advance_day();
    uint8_t get_day() const;
    void set_clock_minutes(uint16_t minutes);
    std::optional<uint16_t> take_desired_clock_minutes();
    void request_clock_restart(uint16_t start_minutes);
    std::optional<uint16_t> take_clock_restart_request();
    void record_sleep_request(uint8_t player_id);
    void answer_sleep(uint8_t player_id, bool accept);
    uint8_t sleeping_count() const;
    bool is_sleeping(uint8_t player_id) const;
    void clear_sleep();
    void end_day();
    void answer_wake(uint8_t player_id);
    bool is_waking() const;
    bool take_day_ended();
    bool take_day_started();

    // --- Centro Comunitario ---
    CommunityCenter& get_community_center();
    const CommunityCenter& get_community_center() const;
    void set_current_room(uint8_t room);
    void leave_dungeon();
    uint8_t get_current_room() const;
    bool donate_to_community_center(uint8_t player_id, const std::string& set_name,
                                    ItemID item, uint16_t quantity);
    bool complete_set(const std::string& set_name);
    bool complete_set(CommunitySet set);
    bool complete_all_sets();
    bool is_set_completed(const std::string& set_name) const;
    bool is_community_center_completed() const;
    void record_enemy_killed(const std::string& kind, uint16_t group_id,
                             uint16_t pair_id, uint16_t room);
    void record_gold_donated(uint16_t amount);
    bool is_market_discount_active() const;
    bool is_double_sell_active() const;
    void activate_market_rewards();
    std::vector<uint8_t> take_completed_sets();
    bool take_victory();

    // --- Regalos ---
    void notify_gift_request(uint8_t sender_id, uint8_t receiver_id, ItemID item_id);
    void notify_gift_answered(uint8_t sender_id);
    void process_gift_acceptance(uint8_t sender_id, uint8_t receiver_id);
    std::vector<GiftOffer> take_gift_requests();
    std::vector<uint8_t> take_gift_answers();
    std::vector<uint8_t> take_accepted_gifts();
    void cancel_gift_request(uint8_t sender_id, uint8_t receiver_id);
};

#endif  // WORLD_H
