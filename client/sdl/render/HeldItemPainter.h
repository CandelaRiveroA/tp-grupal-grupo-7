#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <string>

#include "world/CharacterCatalog.h"
#include "world/CreatureCatalog.h"

namespace world {
class ContentCatalog;
}

namespace sdl {

class SheetCache;

// Dibuja el contenido sostenido sobre la cabeza del personaje (pose
// Carry): el MISMO sprite del contenido seleccionado en el inventario,
// resuelto por contentId en el catálogo de contenidos. Mecanismo
// genérico y único para cualquier contenido (no por herramienta: la
// animación sincronizada de herramientas es ToolOverlayPainter). No
// decide qué dibujar ni cuándo: eso lo resolvió game::ActionResolver.
class HeldItemPainter {
public:
	HeldItemPainter(SheetCache& sheets,
			const world::ContentCatalog& content,
			const world::CharacterCatalog& characters);

	// Dibuja el contenido `contentId` sobre la cabeza de la entidad.
	// No hace nada si el contenido no tiene visual.
	void draw(const std::string& contentId, world::CreatureKind kind,
		  const SDL2pp::Rect& drawBox) const;

private:
	SheetCache& sheets_;
	const world::ContentCatalog& content_;
	const world::CharacterCatalog& characters_;
};

}
