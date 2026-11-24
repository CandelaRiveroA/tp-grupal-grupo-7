#pragma once

#include <string>


// Resumen de un mundo (mapa jugable), para la pantalla de selección de mundo.
struct WorldSummary {
	std::string name;
	// Puede venir vacía.
	std::string description;
};
