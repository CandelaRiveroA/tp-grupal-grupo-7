#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"

struct SavePlayer {
	std::string name;
	CharacterAppearance appearance;
};

// Resumen de una partida guardada, para la pantalla "Retomar".
struct SaveSummary {
	// Identifica la partida al cargarla. No se muestra en la UI.
	std::uint32_t id = 0;
	// Mundo en el que vive la partida. Tampoco se muestra: lo necesita SDL
	// para armar el GameSelection al retomar.
	std::string worldName;
	std::string gameName;
	// En multiplayer hay más de un jugador. El primero es el jugador local.
	std::vector<SavePlayer> players;
	bool multiplayer = false;
	std::uint32_t connectedPlayers = 0;
	std::uint32_t maxPlayers = 0;
	std::string inGameDate;
	std::uint32_t playTimeSeconds = 0;
	std::uint32_t gold = 500;
};
