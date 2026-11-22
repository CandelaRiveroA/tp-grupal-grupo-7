#pragma once

#include <string>

#include "common/model/draft/CharacterAppearance.h"


// Datos que la pantalla de personaje manda al crear la partida.
struct GameSelection {
	std::string username;
	std::string gameName;
	std::string characterName;
	std::string worldName;
	CharacterAppearance appearance;
};
