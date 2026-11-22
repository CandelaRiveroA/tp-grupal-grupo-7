#include "scene.h"

#include <stdexcept>
#include <utility>

Scene::Scene(uint16_t width, uint16_t height):
        width(width), height(height), grid(height) {
    for (uint16_t y = 0; y < this->height; ++y) {
        this->grid[y].resize(this->width);
    }
}

void Scene::on_entity_removed(IEntity* /*entity*/) {}

uint16_t Scene::get_width() const { return this->width; }

uint16_t Scene::get_height() const { return this->height; }

bool Scene::in_bounds(uint16_t x, uint16_t y) const {
    return x < this->width && y < this->height;
}

bool Scene::can_walk_to(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) {
        return false;
    }

    const Cell& target_cell = this->grid[y][x];
    if (target_cell.is_wall) {
        return false;
    }

    // cppcheck-suppress useStlAlgorithm; el loop corta al primer bloqueo y también lee is_wall.
    for (const auto& entity: target_cell.entities) {
        if (!entity->is_walkable()) {
            return false;
        }
    }

    return true;
}

void Scene::set_wall(uint16_t x, uint16_t y, bool wall) {
    if (in_bounds(x, y)) {
        this->grid[y][x].is_wall = wall;
    }
}

const Cell& Scene::get_cell(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    return this->grid[y][x];
}

SoilState Scene::get_soil_state(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) {
        return SoilState::NORMAL;
    }
    return this->grid[y][x].soil_state;
}

std::vector<IEntity*> Scene::get_entity_at(uint16_t x, uint16_t y) const {
    std::vector<IEntity*> found_entities;
    if (!in_bounds(x, y)) {
        return found_entities;
    }

    const Cell& target_cell = this->grid[y][x];
    found_entities.reserve(target_cell.entities.size());
    for (const auto& ent: target_cell.entities) {
        // cppcheck-suppress useStlAlgorithm; la salida es vector<IEntity*>, no una copia de la
        // celda.
        found_entities.push_back(ent.get());
    }

    return found_entities;
}

void Scene::set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> entity) {
    if (!in_bounds(x, y) || !entity) {
        return;
    }

    entity->set_position(x, y);
    this->grid[y][x].entities.push_back(std::move(entity));
}

void Scene::remove_entity(IEntity* target_entity) {
    if (!target_entity) {
        return;
    }

    const uint16_t x = target_entity->get_x();
    const uint16_t y = target_entity->get_y();
    if (!in_bounds(x, y)) {
        return;
    }

    auto& cell_entities = this->grid[y][x].entities;
    for (auto it = cell_entities.begin(); it != cell_entities.end(); ++it) {
        if (it->get() == target_entity) {
            on_entity_removed(target_entity);
            cell_entities.erase(it);
            break;
        }
    }
}

void Scene::remove_entity_at(uint16_t x, uint16_t y) {
    if (!in_bounds(x, y)) {
        return;
    }

    auto& cell_entities = this->grid[y][x].entities;
    if (!cell_entities.empty()) {
        remove_entity(cell_entities.front().get());
    }
}

void Scene::move_entity(IEntity* entity, uint16_t x, uint16_t y) {
    if (!entity || (x == entity->get_x() && y == entity->get_y())) {
        return;
    }
    if (!can_walk_to(x, y)) {
        return;
    }

    const uint16_t from_x = entity->get_x();
    const uint16_t from_y = entity->get_y();
    if (!in_bounds(from_x, from_y)) {
        return;
    }

    std::unique_ptr<IEntity> moved;
    auto& from_entities = this->grid[from_y][from_x].entities;
    for (auto it = from_entities.begin(); it != from_entities.end(); ++it) {
        if (it->get() == entity) {
            moved = std::move(*it);
            from_entities.erase(it);
            break;
        }
    }
    if (!moved) {
        return;
    }

    entity->set_position(x, y);
    this->grid[y][x].entities.push_back(std::move(moved));
}

bool Scene::try_till_soil(uint16_t x, uint16_t y) {
    return false;
}

bool Scene::try_water_soil(uint16_t x, uint16_t y) {
    return false;
}

bool Scene::try_plant_seed(Player& /*p*/, uint16_t /*x*/, uint16_t /*y*/) {
    return false;
}

void Scene::on_new_day(World& /*world*/) {}

const std::vector<std::pair<uint16_t, uint16_t>>& Scene::blood_puddle_list() const {
    static const std::vector<std::pair<uint16_t, uint16_t>> empty;
    return empty;
}

std::vector<ProjectileSnapshot> Scene::projectile_snapshots() const {
    return {};
}
