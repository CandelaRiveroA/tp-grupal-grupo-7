#pragma once

#include "game/PointerClick.h"
#include "sdl/render/SheetCache.h"
#include "world/CursorCatalog.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Pinta el puntero en la capa de pantalla (sin viewport): el sprite se
// alinea por el hotspot del catálogo sobre la posición del mouse. El
// que decide QUÉ variante dibujar es quien llama (leyendo el estado de
// juego); este painter solo dibuja lo que le pasan.
class CursorPainter {
public:
	explicit CursorPainter(SdlContext& context);

	void draw(const world::CursorCatalog& catalog, world::CursorKind kind,
		  const game::PointerPosition& position);

private:
	SheetCache sheets_;
};

}  // namespace sdl
