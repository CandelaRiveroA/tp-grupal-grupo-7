#pragma once

#include <SDL2pp/SDL2pp.hh>

#include "world/CharacterCatalog.h"
#include "world/ContentCatalog.h"
#include "world/ToolOverlay.h"
#include "world/ToolOverlayLayout.h"

namespace game {
struct ViewEntity;
}

namespace sdl {

class SheetCache;

// Dibuja el overlay de lo que la entidad lleva en las manos (herramienta
// en use_tool, el contenido al recoger con pickup, ...) sobre su cuerpo.
// No decide qué dibujar ni en qué frame: esos datos vienen del estado
// (activeTool / activeContent + acción) y de world::ToolOverlayLayout.
// Los sprites salen de dos orígenes: la hoja fija de la herramienta o el
// sprite del contenido sostenido (el mismo del inventario, resuelto por
// contentId con world::ContentCatalog).
class ToolOverlayPainter {
public:
	ToolOverlayPainter(SheetCache& sheets,
			   const world::ToolOverlayCatalog& toolOverlay,
			   const world::CharacterCatalog& characters,
			   const world::ContentCatalog& content);

	void draw(const game::ViewEntity& entity, const SDL2pp::Rect& drawBox,
		  int phase, world::ActionKind action,
		  const world::Pose& pose) const;

private:
	// Herramienta activa: overlay por ItemKind, hoja fija.
	void drawTool(const game::ViewEntity& entity, int phase,
		      world::ActionKind action,
		      const world::ToolOverlayLayout& layout) const;

	// Dibuja los frames de un overlay animado (la herramienta o su
	// efecto): el mismo bucle de capas para ambos, sin duplicar. No
	// decide sincronía ni qué overlay: eso lo resuelve drawTool.
	void drawOverlayFrames(const world::AnimatedOverlay& overlay,
			       int phase,
			       const world::ToolOverlayLayout& layout) const;

	// Contenido sostenido en las manos ([held.*]): overlay común por
	// dirección, dibujando el sprite del contenido que se levanta.
	void drawHeldContent(const game::ViewEntity& entity, int phase,
			     world::ActionKind action,
			     const world::ToolOverlayLayout& layout) const;

	SheetCache& sheets_;
	const world::ToolOverlayCatalog& toolOverlay_;
	const world::CharacterCatalog& characters_;
	const world::ContentCatalog& content_;
};

}  // namespace sdl
