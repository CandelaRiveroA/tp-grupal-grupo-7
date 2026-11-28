#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "sdl/render/PagedModalPainter.h"
#include "world/CommunityBoard.h"
#include "world/CommunityCatalog.h"
#include "world/CommunityPopupStyle.h"

namespace world {
class CharacterCatalog;
class ContentCatalog;
class CropCatalog;
}  // namespace world

namespace sdl {

class InventoryHud;
class SdlContext;
class SheetCache;
class TextRenderer;

// Pinta el tablón paginado del Centro Comunitario y su mini-popup de ayuda,
// reutilizando PagedModalPainter y el SheetCache compartido del Renderer.
class CommunityPopupPainter {
public:
	CommunityPopupPainter(SdlContext& context, InventoryHud& inventoryHud,
			      TextRenderer& text, SheetCache& sheets,
			      const world::ContentCatalog& content,
			      const world::CropCatalog& crops,
			      const world::CharacterCatalog& characters,
			      const world::CommunityCatalog& catalog);

	bool hitsHelpButton(const world::CommunityPopupLayout& layout,
			    const std::string& setTitle, int x, int y);

	void draw(const std::vector<world::CommunitySection>& sections,
		  std::size_t pageIndex, bool helpOpen, int iteration,
		  int screenW, int screenH);

private:
	void drawTopBar(const world::CommunityPopupLayout& layout,
			std::size_t pageIndex, std::size_t pageCount);
	void drawSetHeader(const world::CommunityPopupLayout& layout,
			   const world::CommunitySection& section);
	void drawCard(const world::CommunityPopupLayout& layout,
		      const world::CommunityRow& row,
		      const world::PixelBox& card, int iteration);
	void drawRowSprite(const world::CommunityPopupLayout& layout,
			   const world::CommunityRow& row,
			   const world::PixelBox& card, int iteration);
	void drawCropQualitiesVisual(const world::CommunityPopupLayout& layout,
				     const world::CommunityRow& row,
				     const world::PixelBox& card,
				     int iteration);
	void drawCreatureVisual(const world::CommunityPopupLayout& layout,
				const world::CommunityRow& row,
				const world::PixelBox& card, int iteration);
	void drawCoinVisual(const world::CommunityPopupLayout& layout,
			    const world::CommunityRow& row,
			    const world::PixelBox& card, int iteration);
	void drawContentIconVisual(const world::CommunityPopupLayout& layout,
				   const world::CommunityRow& row,
				   const world::PixelBox& card, int iteration);
	void drawCreature(world::CreatureKind kind, const world::PixelBox& box,
			  int iteration);
	void drawVisual(const world::SpriteVisual& visual,
			const world::PixelBox& box);
	void drawFittedVisual(const world::SpriteVisual& visual,
			      const world::PixelBox& box);
	void drawFooter(const world::CommunityPopupLayout& layout,
			const world::CommunitySection& section,
			std::size_t pageIndex, std::size_t pageCount);
	void drawHelpPopup(const world::CommunityPopupLayout& layout,
			   const world::CommunitySection& section, int screenW,
			   int screenH);

	TextRenderer& text_;
	SheetCache& sheets_;
	const world::ContentCatalog& content_;
	const world::CropCatalog& crops_;
	const world::CharacterCatalog& characters_;
	const world::CommunityCatalog& catalog_;
	PagedModalPainter modalPainter_;
	std::map<world::CommunityVisualKind,
		 void (CommunityPopupPainter::*)(
			 const world::CommunityPopupLayout&,
			 const world::CommunityRow&, const world::PixelBox&,
			 int)>
		visualDrawers_;
	std::optional<SDL2pp::Texture> donateButtonTex_;
	std::optional<SDL2pp::Texture> helpButtonTex_;
};

}  // namespace sdl
