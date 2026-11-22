#pragma once

#include <vector>

#include "sdl/render/SheetCache.h"
#include "world/ContentCatalog.h"
#include "world/MarketPopupStyle.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Pinta el contenido del popup del mercado (títulos, íconos, nombres y
// precios) de las secciones que le pasen, con el estilo compartido. El
// panel lo dibuja InventoryHud; las cajas las calcula
// world::MarketPopupStyle. Qué secciones hay y qué monto trae cada ítem
// ya viene resuelto (world::MarketOfferResolver): esto solo pinta.
class MarketPopupPainter {
public:
	MarketPopupPainter(SdlContext& context, TextRenderer& text);

	void draw(const world::MarketPopupStyle& style,
		  const std::vector<world::MarketSection>& sections,
		  const world::PixelBox& panel,
		  const world::ContentCatalog& content);

private:
	void drawItem(const world::MarketPopupStyle& style,
		      const world::MarketItem& item,
		      const world::PixelBox& cell,
		      const world::ContentCatalog& content);

	TextRenderer& text_;
	SheetCache sheets_;
};

}
