#include "cell.h"

Cell::Cell() : is_wall(false) {}

bool Cell::is_empty() const {
    return this->entities.empty();
}

bool Cell::is_there_a_wall() const {
    return this->is_wall;
}

bool Cell::can_plant_seed() const {
    if (!this->is_soil_tilled() && !this->is_soil_watered()) {
        return false;
    }

    for (const auto& ent : this->entities) {
        if (ent->blocks_placement()) {
            return false;
        }
    }
    return true;
}

bool Cell::is_soil_normal() const {
    return this->soil_state == SoilState::NORMAL;
}

bool Cell::is_soil_tilled() const {
    return this->soil_state == SoilState::TILLED;
}

bool Cell::is_soil_watered() const {
    return this->soil_state == SoilState::WATERED;
}

SoilState Cell::get_soil_state() const {
    return this->soil_state;
}

void Cell::reset_soil() {
    this->soil_state = SoilState::NORMAL;
}

void Cell::till_soil() {
    if (this->soil_state == SoilState::NORMAL) {
        this->soil_state = SoilState::TILLED;
    }
}

void Cell::water_soil() {
    if (this->soil_state == SoilState::TILLED) {
        this->soil_state = SoilState::WATERED;
    }
}

void Cell::dry_soil() {
    if (this->soil_state == SoilState::WATERED) {
        this->soil_state = SoilState::TILLED;
    }
}