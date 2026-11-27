#ifndef SCENE_H
#define SCENE_H

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "../entities/i_entity.h"
#include "cell.h"
#include "flame.h"

class Merchant;

class World;

using SceneId = std::string;

struct MerchantPlacement {
    std::string id;
    uint16_t x;
    uint16_t y;
};

class Scene {
protected:
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<Cell>> grid;
    std::vector<Merchant*> merchants;

    // Hook para que las subclases limpien sus listas de punteros no-dueños
    // (crops, animals, enemies, merchants) justo antes de destruir la entidad.
    virtual void on_entity_removed(IEntity* entity);

public:
    Scene(uint16_t width, uint16_t height);
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    // --- Dimensiones y límites ---
    uint16_t get_width() const;
    uint16_t get_height() const;
    bool in_bounds(uint16_t x, uint16_t y) const;

    // --- Colisiones y celdas ---
    bool can_walk_to(uint16_t x, uint16_t y) const;
    void set_wall(uint16_t x, uint16_t y, bool wall);
    Cell& get_cell(uint16_t x, uint16_t y);
    const Cell& get_cell(uint16_t x, uint16_t y) const;
    SoilState get_soil_state(uint16_t x, uint16_t y) const;

    // --- Gestión de entidades en O(1) ---
    std::vector<IEntity*> get_entity_at(uint16_t x, uint16_t y) const;
    virtual void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> entity);
    void remove_entity(IEntity* target_entity);
    void remove_entity_at(uint16_t x, uint16_t y);
    void move_entity(IEntity* entity, uint16_t x, uint16_t y);
    Merchant* get_merchant_at(uint16_t x, uint16_t y);
    Merchant* get_nearest_merchant(uint16_t x, uint16_t y) const;
    void place_merchant(std::unique_ptr<Merchant> merchant);
    virtual std::vector<MerchantPlacement> get_merchant_placements() const;

    // --- Gestion de acciones sobre el suelo ---
    virtual bool try_till_soil(uint16_t x, uint16_t y);         // devuelve false
    virtual bool try_water_soil(uint16_t x, uint16_t y);        // devuelve false
    virtual bool try_plant_seed(Player& p, uint16_t x, uint16_t y);

    // --- Ciclo de vida ---
    virtual void on_new_day(World& world);

    // --- Lectura para WorldView ---
    virtual const std::vector<std::pair<uint16_t, uint16_t>>& blood_puddle_list() const;
    virtual std::vector<ProjectileSnapshot> projectile_snapshots() const;
};

#endif  // SCENE_H
