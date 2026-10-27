#pragma once

#include <cstdint>
#include <string>


// Resumen de un mundo (mapa jugable), para la pantalla de selección de mundo.
struct WorldSummary {
	std::uint32_t id = 0;
	std::string name;
	// Puede venir vacía.
	std::string description;
};
