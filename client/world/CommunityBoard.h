#pragma once

#include <string>
#include <vector>

namespace world {

// Una fila lista para dibujar del popup del Centro Comunitario: el
// nombre, la cantidad ya formateada ("3 / 10", "$0 / $5000") y si el
// servidor la dio por cumplida (checkbox tildado).
struct CommunityRow {
	std::string label;
	std::string amount;
	bool done = false;
};

// Un set con sus filas. `done` = el servidor lo dio por completo (la
// insignia reemplaza al texto de estado).
struct CommunitySection {
	std::string title;
	std::string status;
	bool done = false;
	std::vector<CommunityRow> rows;
};

}
