#pragma once

#include <cstddef>

#include "sdl/render/ToolOverlayPainter.h"

namespace game {
class ActionResolver;
class ViewMotion;
struct ViewEntity;
}

namespace world {
class CharacterCatalog;
class SceneDescription;
class ToolOverlayCatalog;
}

namespace sdl {

class SheetCache;
class Viewport;

// Dibuja una game::ViewEntity cualquiera: resuelve la apariencia por
// kind, dibuja el cuerpo y delega el overlay de herramienta. No conoce si
// la entidad es el jugador, un enemigo o un animal futuro.
class EntityRenderer {
public:
	EntityRenderer(SheetCache& sheets,
		       const world::CharacterCatalog& characters,
		       const world::ToolOverlayCatalog& toolOverlay);

	void draw(const game::ViewEntity& entity, std::size_t entityIndex,
		  const world::SceneDescription& scene,
		  const game::ViewMotion& motion,
		  const game::ActionResolver& actions, int animationTick,
		  const Viewport& viewport) const;

private:
	SheetCache& sheets_;
	const world::CharacterCatalog& characters_;
	ToolOverlayPainter toolOverlay_;
};

}  // namespace sdl
