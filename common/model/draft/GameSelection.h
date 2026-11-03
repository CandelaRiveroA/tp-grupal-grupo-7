#pragma once

#include <cstdint>
#include <string>

#include "common/model/draft/CharacterAppearance.h"


// Datos que la pantalla de personaje manda al crear la partida.
struct GameSelection {
	// TODO con protocolo: hoy solo lo usa el cliente; copiarlo al MatchRequest cuando viaje.
	std::string username;
	std::string gameName;
	std::string characterName;
	std::uint32_t worldId = 0;
	CharacterAppearance appearance;
};
