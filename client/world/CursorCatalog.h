#pragma once

#include <map>

#include "world/ScenePixelMapper.h"
#include "world/SpriteVisual.h"

namespace world {

// Estado visual del puntero. Agregar una variante = un valor acá + su
// entrada en cursors.toml; el dibujo no cambia.
enum class CursorKind {
	Normal,
	Interactive,
	// Otro jugador (no el local): apunta a "darle un regalo".
	Gift,
	Grab,
};

// Cursores del puntero como DATOS: qué sprite dibuja cada estado y
// desde qué punto "apunta" (hotspot, en px del sprite). No depende de
// SDL; lo carga world::CursorConfigLoader desde ui/cursors.toml y la
// capa SDL lo dibuja. Cuál variante se usa lo decide el estado de
// juego (si hay algo interactivo bajo el cursor), no este catálogo.
struct CursorCatalog {
	std::map<CursorKind, SpriteVisual> visuals;

	// Punto del sprite que se alinea con la posición del puntero.
	PixelPoint hotspot;

	// Una sola búsqueda: nullptr si la variante no está en el TOML.
	const SpriteVisual* find(CursorKind kind) const;
};

}  // namespace world
