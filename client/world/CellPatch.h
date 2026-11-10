#pragma once

#include <cstdint>
#include <set>
#include <utility>
#include <vector>

#include "world/Grid.h"

namespace world {

// Un parche de celdas conectadas: un conjunto de celdas del mundo (la
// tierra trabajada, el agua, un camino, ...) con la consulta de
// conectividad que necesita el auto-tiling: qué vecinos cardinales de
// una celda están también en el parche (la máscara de bits
// kNeighbor* de WorldConstants.h).
//
// ES EL ALGORITMO, solo el algoritmo: no conoce sprites, ni
// consecuencias, ni dibujo. Cualquier capa que se arme por zonas
// conectadas (regado, caminos, ...) construye su CellPatch y consulta
// las mismas máscaras (§9: geometría separada del pintado).
class CellPatch {
public:
	explicit CellPatch(const std::vector<GridCell>& cells);

	bool contains(const GridCell& cell) const;

	// Máscara de los vecinos cardinales presentes en el parche
	// (kNeighborNorth | kNeighborEast | ...). La celda misma no
	// necesita estar en el parche.
	std::uint8_t maskAt(const GridCell& cell) const;

private:
	// Presencia por celda (col, row) para la consulta por vecinos.
	std::set<std::pair<int, int>> cells_;
};

}
