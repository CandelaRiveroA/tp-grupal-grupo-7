#include "Renderer.h"

#include "core/Constants.h"
#include "game/ActionResolver.h"
#include "sdl/context/SdlConstants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/ElementPainter.h"
#include "world/EntityAppearance.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

Renderer::Renderer(SdlContext& context,
		   const world::CharacterCatalog& characters)
	: context_(context)
	, catalog_(characters)
	, characterSheet_(context,
			  core::assetPath(characters.sheetFile(
				  world::CreatureKind::Player)),
			  characters.sheetCellWidth(world::CreatureKind::Player),
			  characters.sheetCellHeight(world::CreatureKind::Player))
	, character_(characterSheet_,
		     characters.drawnWidth(world::CreatureKind::Player),
		     characters.drawnHeight(world::CreatureKind::Player))
	, sheetCache_(context) {}

void Renderer::render(const world::SceneDescription& scene,
		      const game::LocalWorldView& view,
		      const game::ViewMotion& motion,
		      const game::ActionResolver& actionResolver,
		      int animationTick) {
	renderScene(scene, view, motion, actionResolver, animationTick);
	context_.present();
}

void Renderer::renderScene(const world::SceneDescription& scene,
			   const game::LocalWorldView& view,
			   const game::ViewMotion& motion,
			   const game::ActionResolver& actionResolver,
			   int animationTick) {

	const world::Color& background = scene.floorColor;
	context_.clear(SDL2pp::Color(background.r, background.g,
				     background.b, background.a));

	for (const world::ElementPlacement& element : scene.elements) {
		drawElement(scene, element);
	}

	for (std::size_t i = 0; i < view.entities.size(); ++i) {
		drawEntity(scene, view.entities[i], motion, actionResolver, i,
			   animationTick);
	}
}

void Renderer::drawEntity(const world::SceneDescription& scene,
			  const game::ViewEntity& entity,
			  const game::ViewMotion& motion,
			  const game::ActionResolver& actionResolver,
			  std::size_t entityIndex, int animationTick) {
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect box =
		mapper.toPixels(world::GridCell{entity.cellX, entity.cellY},
				world::GridSpan{1, 1});

	const int phase =
		actionResolver.phaseFor(entity, entityIndex, animationTick);

	const world::ActionKind action =
		actionResolver.actionFor(entity, entityIndex, animationTick);

	const world::EntityAppearance appearance = world::appearanceFor(
		catalog_.dataFor(entity.kind), entity.facing, action, phase);

	const game::PixelOffset offset = motion.offsetFor(
		entityIndex, animationTick, scene.cellPixelSize);

	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;
	character_.draw(*appearance.pose,
			SDL2pp::Rect(box.x + offset.x, box.y + offset.y,
				     box.width, box.height),
			flip);
}

void Renderer::drawElement(const world::SceneDescription& scene,
			   const world::ElementPlacement& element) {

	const world::TileSetDef& tileset =
		scene.tilesets.at(element.tileset);

	SpriteSheet& sheet = sheetCache_.sheet(
		element.tileset, core::assetPath(tileset.image),
		tileset.cellWidth, tileset.cellHeight);

	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect area =
		mapper.toPixels(element.at, element.span);

	ElementPainter(tileset, element, area).paint(sheet);
}

}
