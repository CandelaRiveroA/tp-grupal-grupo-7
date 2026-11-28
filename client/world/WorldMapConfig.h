#pragma once

#include <string>
#include <vector>

#include "world/Color.h"
#include "world/PagedModal.h"

namespace world {

struct WorldMapExteriorConfig {
	std::string id;
	std::string title;
};

// Configuración de presentación del mapa del mundo cargada desde el TOML.
// Reutiliza PagedModalSpec para el contenedor, flechas y colores base.
struct WorldMapConfig {
	explicit WorldMapConfig(const std::string& path);

	PagedModalSpec modal;
	int titleTopPad = 0;
	int titleHeight = 0;
	int titleFontSize = 0;
	int mapTopGap = 0;
	int mapBorder = 0;
	std::string unavailableText;
	int unavailableFontSize = 0;
	int pinSize = 0;
	int pinBorder = 0;
	Color localRingColor{};
	std::vector<WorldMapExteriorConfig> exteriors;
};

}  // namespace world
