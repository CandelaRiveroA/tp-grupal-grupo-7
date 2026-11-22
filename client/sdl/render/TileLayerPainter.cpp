#include "sdl/render/TileLayerPainter.h"

#include <SDL2pp/Rect.hh>

#include "core/Constants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/TileLayerLayout.h"

namespace sdl {

TileLayerPainter::TileLayerPainter(SheetCache& sheets) : sheets_(sheets) {}

void TileLayerPainter::draw(const world::SceneDescription& scene,
			    const world::ScenePixelMapper& mapper,
			    const Viewport& viewport) {
	for (const world::TileLayer& layer : scene.layers) {
		drawLayer(layer, scene, mapper, viewport);
	}
}

void TileLayerPainter::drawLayer(const world::TileLayer& layer,
				 const world::SceneDescription& scene,
				 const world::ScenePixelMapper& mapper,
				 const Viewport& viewport) {
	const world::TileLayerLayout layout(mapper);
	const world::TileLayerLayout::Range range =
		layout.visible(layer.size, viewport.visibleWorldRect());
	if (range.empty()) {
		return;
	}
	for (int row = range.firstRow; row <= range.lastRow; ++row) {
		for (int column = range.firstColumn; column <= range.lastColumn;
		     ++column) {
			const world::LayerCell& cell =
				layer.cells[static_cast<std::size_t>(
					row * layer.size.columns + column)];
			if (cell.tilesetIndex < 0) {
				continue;
			}
			const std::string& name =
				layer.tilesets[static_cast<std::size_t>(
					cell.tilesetIndex)];
			const world::TileSetDef& tileset = scene.tilesets.at(name);
			SpriteSheet& sheet = sheets_.sheet(
				name, core::assetPath(tileset.image),
				tileset.cellWidth, tileset.cellHeight);
			const world::PixelRect dest =
				viewport.toScreen(layout.cellRect(column, row));
			sheet.drawBlock(cell.sheetCell.column, cell.sheetCell.row, 1,
					1,
					SDL2pp::Rect(dest.x, dest.y, dest.width,
						     dest.height));
		}
	}
}

}  // namespace sdl
