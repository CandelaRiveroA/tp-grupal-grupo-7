#pragma once

namespace world {

struct GridCell {
	int column;
	int row;

	// Orden total (fila y columna): permite usar celdas como clave de
	// map (p. ej. el reloj de los diálogos de los cultivos).
	bool operator<(const GridCell& other) const;
};

struct GridSpan {
	int columns;
	int rows;
};

}
