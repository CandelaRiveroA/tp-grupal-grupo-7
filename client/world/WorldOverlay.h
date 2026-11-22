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

// La decoración es "entrable" (la cama): las filas de su pieza que
// tapan a quien se mete, hacia dónde mira y qué marker del mapa marca
// la celda donde queda acostado. Mismo concepto que `enter` de
// scenes.toml, pero la celda sale del mapa (la misma que usa el server).
struct OverlayEnter {
	int firstFrontRow;
	int lastFrontRow;
	Direction facing;
	std::string marker;
};

// Lo que el cliente agrega a una decoración que puso el editor.
struct DecorationOverlay {
	std::optional<NightVariant> night;
	std::optional<OverlayEnter> enter;
};

// Qué hace el click sobre un edificio de cierto rol.
struct RoleBehavior {
	StructureAction action = StructureAction::None;
	std::string target;
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
	// Qué NPC (id de npcs.toml) se ubica en cada kind de marker
	// ("merchant_pierre" -> "pierre"): su celda por defecto.
	std::map<std::string, std::string> markerNpcs;
	// Comportamiento del click por rol del editor ("market",
	// "community_center", ...).
	std::map<std::string, RoleBehavior> roles;
	// Decoraciones iniciales por id de mapa, en orden de dibujo.
	std::map<std::string, std::vector<ElementPlacement>> mapElements;
};

}
