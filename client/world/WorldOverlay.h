#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/Grid.h"
#include "world/SceneDescription.h"
#include "world/WorldConstants.h"

namespace world {

// Versión de noche de una decoración: misma pieza (mismo tamaño), con su
// origen en esta celda de esta hoja.
struct NightVariant {
	std::string tileset;
	GridCell sheetCell;
};

// Lo que el cliente agrega a una decoración que puso el editor.
struct DecorationOverlay {
	std::optional<NightVariant> night;
};

// Datos del cliente que completan el mundo del editor (overlay.toml):
// DATOS puros, sin SDL. El editor decide DÓNDE está cada cosa; esto
// dice cómo se escala, cómo cambia de noche y qué decoraciones iniciales
// trae cada mapa (lo que el cliente pone en el primer arranque).
struct WorldOverlay {
	// Px de pantalla por px de arte, uno solo para todo el mapa.
	int scale = kDefaultWorldScale;
	// Hojas propias del cliente (decoraciones iniciales, variantes).
	std::map<std::string, TileSetDef> tilesets;
	// Por tile de origen ("tileset:col,row").
	std::map<std::string, DecorationOverlay> decorations;
	// Decoraciones iniciales por id de mapa, en orden de dibujo.
	std::map<std::string, std::vector<ElementPlacement>> mapElements;
};

}
