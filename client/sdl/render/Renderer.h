#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "game/ViewSpatialIndex.h"
#include "sdl/render/EntityRenderer.h"
#include "sdl/render/CropPainter.h"
#include "sdl/render/GroundEffectPainter.h"
#include "sdl/render/SheetCache.h"
#include "world/ContentCatalog.h"
#include "world/CropCatalog.h"
#include "world/GroundCatalog.h"
#include "world/ResourceCatalog.h"
#include "world/SceneDescription.h"
#include "world/SceneElementIndex.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class SdlContext;
class Viewport;
}

namespace sdl {

// Orquestador del frame de mundo: limpia, consulta qué está visible y delega
// cada capa de dibujo en su pintor. No conoce cómo se dibuja una entidad.
class Renderer {
public:
	Renderer(SdlContext& context, const world::CharacterCatalog& characters,
		 const world::ToolOverlayCatalog& toolOverlay,
		 const world::ResourceCatalog& resources,
		 const world::ContentCatalog& content,
		 const world::GroundCatalog& ground,
		 const world::CropCatalog& crops);

	// Reconstruye el índice de elementos estáticos. Se llama cuando se
	// entra a una escena o cuando cambia su contenido, no por frame.
	void prepareScene(const world::SceneDescription& scene);

	void drawWorld(const world::SceneDescription& scene,
		       const game::LocalWorldView& view,
		       const game::ViewMotion& motion,
		       const game::ActionResolver& actionResolver,
		       int animationTick, const Viewport& viewport);

private:
	void drawResource(const game::ViewResource& resource,
			  const world::ScenePixelMapper& mapper,
			  const Viewport& viewport);

	void drawDroppedItem(const game::ViewDroppedItem& item,
			     const world::ScenePixelMapper& mapper,
			     const Viewport& viewport);

	void drawElement(const world::SceneDescription& scene,
			 const world::ElementPlacement& element,
			 const world::ScenePixelMapper& mapper,
			 const Viewport& viewport);

	SdlContext& context_;

	const world::ResourceCatalog& resources_;
	const world::ContentCatalog& content_;
	const world::GroundCatalog& ground_;

	SheetCache sheetCache_;
	EntityRenderer entityRenderer_;
	GroundEffectPainter groundPainter_;
	CropPainter cropPainter_;

	world::SceneElementIndex sceneIndex_;
	game::ViewSpatialIndex viewIndex_;
	bool sceneIndexReady_ = false;

	std::vector<std::uint32_t> visibleElements_;
	std::vector<std::uint32_t> visibleResources_;
	std::vector<std::uint32_t> visibleItems_;
	std::vector<std::uint32_t> visibleEntities_;
};

}  // namespace sdl
