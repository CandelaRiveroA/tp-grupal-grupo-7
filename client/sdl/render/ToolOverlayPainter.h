#pragma once

#include <SDL2pp/SDL2pp.hh>

#include "world/CharacterCatalog.h"
#include "world/ToolOverlay.h"

namespace game {
struct ViewEntity;
}

namespace sdl {

class SheetCache;

// Dibuja el overlay de la herramienta activa sobre el cuerpo de una entidad.
// No decide qué herramienta ni qué frame: esos datos vienen del estado y de
// world::ToolOverlayLayout.
class ToolOverlayPainter {
public:
	ToolOverlayPainter(SheetCache& sheets,
			   const world::ToolOverlayCatalog& toolOverlay,
			   const world::CharacterCatalog& characters);

	void draw(const game::ViewEntity& entity, const SDL2pp::Rect& drawBox,
		  int phase, world::ActionKind action) const;

private:
	SheetCache& sheets_;
	const world::ToolOverlayCatalog& toolOverlay_;
	const world::CharacterCatalog& characters_;
};

}  // namespace sdl
