#include "sdl/render/Renderer.h"

#include "core/Constants.h"
#include "game/ActionResolver.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/ElementPainter.h"
#include "sdl/render/RenderConstants.h"
#include "sdl/render/Viewport.h"
#include "world/ScenePixelMapper.h"
#include "world/WorldConstants.h"

namespace sdl {

Renderer::Renderer(SdlContext& context,
		   const world::CharacterCatalog& characters,
		   const world::ToolOverlayCatalog& toolOverlay,
		   const world::ResourceCatalog& resources,
		   const world::ContentCatalog& content)
	: context_(context)
	, resources_(resources)
	, content_(content)
	, sheetCache_(context)
	, entityRenderer_(sheetCache_, characters, toolOverlay) {}

void Renderer::prepareScene(const world::SceneDescription& scene) {
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	sceneIndex_.rebuild(scene, mapper);
	sceneIndexReady_ = true;
}

void Renderer::drawWorld(const world::SceneDescription& scene,
			 const game::LocalWorldView& view,
			 const game::ViewMotion& motion,
			 const game::ActionResolver& actionResolver,
			 const int animationTick,
			 const Viewport& viewport) {
	const world::Color& background = scene.floorColor;
	context_.clear(SDL2pp::Color(background.r, background.g,
				     background.b, background.a));

	if (!sceneIndexReady_) {
		prepareScene(scene);
	}

	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect visibleWorld = viewport.visibleWorldRect();
	const world::PixelRect dynamicQuery{
		visibleWorld.x - render::kSpriteOverdrawMarginPixels,
		visibleWorld.y - render::kSpriteOverdrawMarginPixels,
		visibleWorld.width + render::kSpriteOverdrawMarginPixels * 2,
		visibleWorld.height + render::kSpriteOverdrawMarginPixels * 2,
	};

	viewIndex_.rebuild(view, mapper);

	sceneIndex_.query(visibleWorld, visibleElements_);
	viewIndex_.queryResources(dynamicQuery, visibleResources_);
	viewIndex_.queryDroppedItems(dynamicQuery, visibleItems_);
	viewIndex_.queryEntities(dynamicQuery, visibleEntities_);

	for (const std::uint32_t index : visibleElements_) {
		drawElement(scene, scene.elements[index], mapper, viewport);
	}

	// Recursos antes que las entidades: el personaje se dibuja encima.
	for (const std::uint32_t index : visibleResources_) {
		drawResource(view.resources[index], mapper, viewport);
	}

	// Minerales caídos al piso: también debajo de las entidades, para que
	// el personaje pase por encima al pararse sobre ellos.
	for (const std::uint32_t index : visibleItems_) {
		drawDroppedItem(view.droppedItems[index], mapper, viewport);
	}

	for (const std::uint32_t index : visibleEntities_) {
		entityRenderer_.draw(view.entities[index], index, scene, motion,
				    actionResolver, animationTick, viewport);
	}
}

void Renderer::drawElement(const world::SceneDescription& scene,
			   const world::ElementPlacement& element,
			   const world::ScenePixelMapper& mapper,
			   const Viewport& viewport) {
	const world::TileSetDef& tileset =
		scene.tilesets.at(element.tileset);

	SpriteSheet& sheet = sheetCache_.sheet(
		element.tileset, core::assetPath(tileset.image),
		tileset.cellWidth, tileset.cellHeight);

	const world::PixelRect area =
		viewport.toScreen(mapper.toPixels(element.at, element.span));

	const ElementPainter painter(tileset, element, area);
	if (!viewport.intersects(painter.bounds())) {
		return;
	}
	painter.paint(sheet, viewport.screenRect());
}

void Renderer::drawResource(const game::ViewResource& resource,
			    const world::ScenePixelMapper& mapper,
			    const Viewport& viewport) {
	const world::ResourceVisual* found =
		resources_.find(resource.kind, resource.state);
	if (found == nullptr) {
		return;
	}
	const world::ResourceVisual& visual = *found;

	const world::PixelRect cell = viewport.toScreen(mapper.toPixels(
		world::GridCell{resource.cellX, resource.cellY},
		world::kUnitCellSpan));

	// Anclado a la base de la celda: al achicarse se desmorona desde
	// arriba. scale agranda para tapar el aire transparente de arriba.
	const int baseW = cell.width;
	const int baseH = (visual.cols > 0)
		? baseW * visual.rows / visual.cols
		: cell.height;
	const int w = static_cast<int>(baseW * resources_.scale);
	const int h = static_cast<int>(baseH * resources_.scale);
	const int x = cell.x + (cell.width - w) / 2;
	const int y = cell.y + cell.height - h + resources_.offsetY;

	const world::PixelRect drawRect{x, y, w, h};
	if (!viewport.intersects(drawRect)) {
		return;
	}

	SpriteSheet& sheet = sheetCache_.sheet(
		visual.sprite.sheetImage,
		core::assetPath(visual.sprite.sheetImage),
		visual.sprite.cellWidth, visual.sprite.cellHeight);

	sheet.drawBlock(visual.sprite.col, visual.sprite.row, visual.cols,
			visual.rows,
			SDL2pp::Rect(x, y, w, h));
}

void Renderer::drawDroppedItem(const game::ViewDroppedItem& item,
			       const world::ScenePixelMapper& mapper,
			       const Viewport& viewport) {
	// El item en el piso se dibuja con el MISMO asset que en el slot del
	// inventario (ContentCatalog, por contentId). Así un mineral, un
	// huevo o cualquier recolectable futuro usan su celda sin casos
	// especiales. Más chico que la celda y centrado, para que se lea
	// como un item apoyado en el suelo.
	const world::SpriteVisual* found = content_.find(item.contentId);
	if (found == nullptr) {
		return;
	}
	const world::SpriteVisual& visual = *found;

	const world::PixelRect cell = viewport.toScreen(mapper.toPixels(
		world::GridCell{item.cellX, item.cellY},
		world::kUnitCellSpan));

	// Tamaño en el mapa: `drawn` si el TOML lo declara (sprites
	// únicos como el huevo o la leche), si no una fracción de la celda
	// para que quede como un item apoyado en el piso.
	const int baseW = (visual.drawnWidth > 0)
		? visual.drawnWidth
		: static_cast<int>(cell.width * render::kFloorItemScale);
	const int baseH = (visual.drawnHeight > 0)
		? visual.drawnHeight
		: static_cast<int>(cell.height * render::kFloorItemScale);
	const int x = cell.x + (cell.width - baseW) / 2;
	const int y = cell.y + cell.height - baseH + resources_.offsetY;

	const world::PixelRect drawRect{x, y, baseW, baseH};
	if (!viewport.intersects(drawRect)) {
		return;
	}

	SpriteSheet& sheet = sheetCache_.sheet(
		visual.sheetImage, core::assetPath(visual.sheetImage),
		visual.cellWidth, visual.cellHeight);

	sheet.draw(visual.col, visual.row,
		   SDL2pp::Rect(x, y, baseW, baseH));
}

}  // namespace sdl
