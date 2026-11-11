#ifndef CELL_H
#define CELL_H

#include <memory>
#include <vector>
#include "../entities/i_entity.h"

//TODO: Una entidad no traspasable no pude vivir con otras entidades
// Hacer filtra para que si la entidad no es traspasable, no se pueda agregar otra entidad a la celda
enum class SoilState { 
    NORMAL,    // Pasto/tierra dura
    TILLED,    // Arada (cavada), lista para semilla
    WATERED    // Regada
};

struct Cell {
    bool is_wall; 
    SoilState soil_state = SoilState::NORMAL;
    std::vector<std::unique_ptr<IEntity>> entities;

    Cell();
    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell(Cell&&) = default;
    Cell& operator=(Cell&&) = default;

    bool is_empty() const;
    bool is_there_a_wall() const;
    bool can_plant_seed() const;

    bool is_soil_normal() const;
    bool is_soil_tilled() const;
    bool is_soil_watered() const;

    void reset_soil();
    void till_soil();
    void water_soil();
    void dry_soil();

};

#endif // CELL_H
