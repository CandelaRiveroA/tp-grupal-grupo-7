#pragma once

#include <cstddef>

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "sdl/render/CharacterSprite.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/CharacterCatalog.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class Renderer {
public:
	Renderer(SdlContext& context, const world::CharacterCatalog& characters);

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

	void drawEntity(const world::SceneDescription& scene,
			const game::ViewEntity& entity,
			const game::ViewMotion& motion,
			const game::ActionResolver& actionResolver,
			std::size_t entityIndex, int animationTick);

	void drawElement(const world::SceneDescription& scene,
			 const world::ElementPlacement& element);

	SheetCache sheetCache_;
};

}
