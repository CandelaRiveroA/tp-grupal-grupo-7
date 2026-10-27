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
		   const world::CharacterCatalog& characters,
		   const world::ItemCatalog& itemCatalog)
	: context_(context)
	, catalog_(characters)
	, itemCatalog_(itemCatalog)
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

	// Orden de dibujo: elementos -> ítems del suelo -> criaturas
	drawGroundItems(scene, view);

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
	
	SpriteSheet& sheet = sheetCache_.sheet(
		catalog_.sheetFile(entity.kind),
		core::assetPath(catalog_.sheetFile(entity.kind)),
		catalog_.sheetCellWidth(entity.kind),
		catalog_.sheetCellHeight(entity.kind));
	const CharacterSprite sprite(
		sheet, catalog_.drawnWidth(entity.kind),
		catalog_.drawnHeight(entity.kind));

	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;
	sprite.draw(*appearance.pose,
		    SDL2pp::Rect(box.x + offset.x, box.y + offset.y,
				 box.width, box.height),
		    flip);
}

void Renderer::drawGroundItems(const world::SceneDescription& scene,
			       const game::LocalWorldView& view) {
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	for (const game::ViewGroundItem& item : view.groundItems) {
		if (!itemCatalog_.has(item.item)) {
			// Ítem sin visual en el TOML: se ignora, igual
			// que en el inventario (sin error).
			continue;
		}
		const world::ItemVisual& visual =
			itemCatalog_.visualFor(item.item);

		SpriteSheet& sheet = sheetCache_.sheet(
			visual.sheetImage, core::assetPath(visual.sheetImage),
			visual.cellWidth, visual.cellHeight);

		const world::PixelRect cell = mapper.toPixels(
			world::GridCell{item.cellX, item.cellY},
			world::GridSpan{1, 1});
		// Tamaño en el mapa: `drawn` si el TOML lo declara;
		// si no, el nativo de la celda de la hoja.
		const int width = visual.drawnWidth > 0
				      ? visual.drawnWidth
				      : visual.cellWidth;
		const int height = visual.drawnHeight > 0
				       ? visual.drawnHeight
				       : visual.cellHeight;
		// Centrado en su celda: un ítem del suelo no ancla abajo
		// como una criatura (no "para" en el piso).
		const int x = cell.x + (cell.width - width) / 2;
		const int y = cell.y + (cell.height - height) / 2;
		sheet.draw(visual.col, visual.row,
			   SDL2pp::Rect(x, y, width, height));
	}
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
