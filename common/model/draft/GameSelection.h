#pragma once

#include <cstdint>
#include <string>

#include "common/model/draft/CharacterAppearance.h"


// Lo que el lobby entrega a SDL al confirmar.
struct GameSelection {
	std::string gameName;
	std::string characterName;
	std::uint32_t worldId = 0;
	CharacterAppearance appearance;
};
