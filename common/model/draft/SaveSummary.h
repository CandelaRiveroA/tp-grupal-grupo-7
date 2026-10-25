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
// En multiplayer hay más de un jugador.
struct SaveSummary {
	std::string gameName;
	std::vector<SavePlayer> players;
	bool multiplayer = false;
	std::string inGameDate;
	std::uint32_t playTimeSeconds = 0;
};
