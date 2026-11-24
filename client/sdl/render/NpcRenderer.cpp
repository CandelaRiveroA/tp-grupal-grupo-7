#include "sdl/render/NpcRenderer.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"

namespace sdl {

NpcRenderer::NpcRenderer(SdlContext& context) : sheets_(context) {}

void NpcRenderer::setNpcs(const std::vector<const world::NpcDef*>& npcs) {
	npcs_ = npcs;
}

void NpcRenderer::setFrames(const std::map<std::string, int>& frames) {
	frames_ = frames;
}

void NpcRenderer::draw(const Viewport& viewport, const int cellPixelSize,
		       const world::NpcCells& placed) {
	for (const world::NpcDef* npc : npcs_) {
		if (npc == nullptr) {
			continue;
		}
		const world::GridCell where = npc->cellIn(placed);
		const world::PixelRect body =
			npc->body.box(cellPixelSize, where);
		if (!viewport.intersects(viewport.toScreen(body))) {
			continue;
		}
		const auto animated = frames_.find(npc->id);
		const int frame = animated != frames_.end() ? animated->second : 0;
		SpriteSheet& sheet = sheets_.sheet(
			npc->body.sheet, core::assetPath(npc->body.sheet),
			npc->body.cellWidth, npc->body.cellHeight);
		for (const world::NpcBodyPiece& piece :
		     npc->body.pieces(cellPixelSize, where, frame)) {
			const world::PixelRect dest =
				viewport.toScreen(piece.dest);
			sheet.drawBlock(piece.cell.column, piece.cell.row, 1, 1,
					SDL2pp::Rect(dest.x, dest.y,
						     dest.width, dest.height));
		}
	}
}

}
