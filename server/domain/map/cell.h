#ifndef CELL_H
#define CELL_H

#include <memory>
#include <vector>
#include "../entities/i_entity.h"

//TODO: Una entidad no traspasable no pude vivir con otras entidades
// Hacer filtra para que si la entidad no es traspasable, no se pueda agregar otra entidad a la celda

struct Cell {
    bool is_wall; 
    std::vector<std::unique_ptr<IEntity>> entities;

    Cell();
    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell(Cell&&) = default;
    Cell& operator=(Cell&&) = default;

    bool is_empty() const;
};

#endif // CELL_H
