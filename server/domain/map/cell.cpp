#include "cell.h"

Cell::Cell() : is_wall(false) {}

bool Cell::is_empty() const {
    return this->entities.empty();
}

bool Cell::is_there_a_wall() const {
    return this->is_wall;
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