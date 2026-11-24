#pragma once

#include <string>

#include "world/SceneDescription.h"

namespace world {

// Regla de coherencia entre el arte y la grilla: la celda de arte de una
// hoja debe entrar EXACTA en la celda lógica de la escena (nunca se
// deforma). Devuelve la escala de dibujo que lo logra.
class ArtCellFit {
public:
	// Lanza ConfigError(InvalidValue, "tilesets.<name>.cell") si la hoja
	// no divide exacto la celda lógica.
	int scaleFor(const TileSetDef& tileset, int cellPixelSize,
		     const std::string& tilesetName) const;
};

}
