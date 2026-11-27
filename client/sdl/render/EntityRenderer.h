#pragma once

#include <cstddef>

#include "sdl/render/HeldItemPainter.h"
#include "sdl/render/PlayerAppearancePainter.h"
#include "sdl/render/ToolOverlayPainter.h"

namespace game {
class ActionResolver;
class ViewMotion;
struct ViewEntity;
}

namespace world {
class CharacterCatalog;
class ContentCatalog;
class SceneDescription;
class ToolOverlayCatalog;
struct CharacterPalettes;
}

namespace sdl {

class SdlContext;
class SheetCache;
class Viewport;

// Dibuja una game::ViewEntity cualquiera: resuelve la apariencia por
// kind, dibuja el cuerpo y delega los overlays (herramienta en uso y
// contenido sostenido). No conoce si la entidad es el jugador, un
// enemigo o un animal futuro, y no decide QUÉ se dibuja: anima la
// acción que resuelve game::ActionResolver.
class EntityRenderer {
public:
	EntityRenderer(SdlContext& context, SheetCache& sheets,
		       const world::CharacterCatalog& characters,
		       const world::ToolOverlayCatalog& toolOverlay,
		       const world::ContentCatalog& content,
		       const world::CharacterPalettes& palettes);

	void draw(const game::ViewEntity& entity, std::size_t entityIndex,
		  const world::SceneDescription& scene,
		  const game::ViewMotion& motion,
		  const game::ActionResolver& actions, int animationTick,
		  const Viewport& viewport) const;

	const PlayerAppearancePainter& appearancePainter() const;

private:
	SheetCache& sheets_;
	const world::CharacterCatalog& characters_;
	PlayerAppearancePainter appearancePainter_;
	ToolOverlayPainter toolOverlay_;
	HeldItemPainter heldItem_;
};

}  // namespace sdl
