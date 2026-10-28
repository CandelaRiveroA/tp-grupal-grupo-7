#pragma once

#include <cstddef>

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "sdl/render/CharacterSprite.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/CharacterCatalog.h"
#include "world/ContentCatalog.h"
#include "world/ResourceCatalog.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"
#include "world/ToolOverlay.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class Renderer {
public:
	Renderer(SdlContext& context, const world::CharacterCatalog& characters,
		 const world::ToolOverlayCatalog& toolOverlay,
		 const world::ResourceCatalog& resources,
		 const world::ContentCatalog& content);

	void render(const world::SceneDescription& scene,
		    const game::LocalWorldView& view,
		    const game::ViewMotion& motion,
		    const game::ActionResolver& actionResolver,
		    int animationTick);

	void renderScene(const world::SceneDescription& scene,
			 const game::LocalWorldView& view,
			 const game::ViewMotion& motion,
			 const game::ActionResolver& actionResolver,
			 int animationTick);

private:
	SdlContext& context_;

	const world::CharacterCatalog& catalog_;
	const world::ToolOverlayCatalog& toolOverlay_;
	const world::ResourceCatalog& resources_;
	const world::ContentCatalog& content_;

	void drawEntity(const world::SceneDescription& scene,
			const game::ViewEntity& entity,
			const game::ViewMotion& motion,
			const game::ActionResolver& actionResolver,
			std::size_t entityIndex, int animationTick);

	void drawToolOverlay(const game::ViewEntity& entity,
			     const SDL2pp::Rect& drawBox, int phase,
			     world::ActionKind action);

	void drawResource(const world::SceneDescription& scene,
			  const game::ViewResource& resource);

	void drawDroppedItem(const world::SceneDescription& scene,
			     const game::ViewDroppedItem& item);

	void drawElement(const world::SceneDescription& scene,
			 const world::ElementPlacement& element);

	SpriteSheet characterSheet_;
	CharacterSprite character_;

	SheetCache sheetCache_;
};

}
