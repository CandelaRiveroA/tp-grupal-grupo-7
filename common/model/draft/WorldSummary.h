#pragma once

#include <cstdint>
#include <string>


// Resumen de un mundo (mapa jugable), para la pantalla de selección de mundo.
struct WorldSummary {
	std::string name;
	// Puede venir vacía.
	std::string description;
	// 0 indica que el servidor aún no informó el límite.
	std::uint16_t maxPlayers = 0;
};
