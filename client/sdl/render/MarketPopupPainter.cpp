#include "sdl/render/MarketPopupPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include <algorithm>
#include <optional>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

MarketPopupPainter::MarketPopupPainter(SdlContext& context,
				       TextRenderer& text)
	: text_(text), sheets_(context) {}

void MarketPopupPainter::draw(const world::MarketPopupStyle& style,
			      const std::vector<world::MarketSection>& sections,
			      const world::PixelBox& panel,
			      const world::ContentCatalog& content) {
	for (std::size_t s = 0; s < sections.size(); ++s) {
		text_.drawLeftWrapped(sections[s].title,
				      style.titleBox(panel, sections, s),
				      style.titleColor, style.titleSize, 0);
		for (std::size_t i = 0; i < sections[s].items.size(); ++i) {
			drawItem(style, sections[s].items[i],
				 style.cellBox(panel, sections, s, i), content);
		}
	}
}

void MarketPopupPainter::drawItem(const world::MarketPopupStyle& style,
				  const world::MarketItem& item,
				  const world::PixelBox& cell,
				  const world::ContentCatalog& content) {
	const std::optional<int>& price = item.price;

	if (const world::SpriteVisual* visual = content.find(item.contentId)) {
		// El ícono entra en su caja sin deformarse.
		const world::PixelBox box = visual->fitIn(style.iconBox(cell));
		SpriteSheet& sheet = sheets_.sheet(
			visual->sheetImage, core::assetPath(visual->sheetImage),
			visual->cellWidth, visual->cellHeight);
		sheet.drawBlock(visual->col, visual->row, visual->cols,
				visual->rows,
				SDL2pp::Rect(box.x, box.y, box.width,
					     box.height));
	}

	text_.drawLeftWrapped(item.label,
			      style.labelBox(cell, price.has_value()),
			      style.textColor, style.textSize, 0);
	if (price.has_value()) {
		const world::SpriteVisual& coin = style.coin;
		const world::PixelBox coinBox = style.coinBox(cell);
		sheets_.sheet(coin.sheetImage, core::assetPath(coin.sheetImage),
			      coin.cellWidth, coin.cellHeight)
			.draw(coin.col, coin.row,
			      SDL2pp::Rect(coinBox.x, coinBox.y, coinBox.width,
					   coinBox.height));
		text_.drawLeftWrapped(std::to_string(*price),
				      style.priceBox(cell), style.priceColor,
				      style.textSize, 0);
	}
}

}
