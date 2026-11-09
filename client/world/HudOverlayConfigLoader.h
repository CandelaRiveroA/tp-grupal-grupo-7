#pragma once

#include <string>

#include "world/HudOverlayCatalog.h"

namespace world {

// Carga ui/hud.toml hacia un HudOverlayCatalog. Mismo patrón que
// DigitConfigLoader / CursorConfigLoader: TomlDocument -> reader interno
// -> datos puros. Lanza ConfigError si el archivo es inválido.
class HudOverlayConfigLoader {
public:
	HudOverlayCatalog loadFromFile(const std::string& path) const;

	HudOverlayCatalog loadFromString(const std::string& toml) const;
};

}
