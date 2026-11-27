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
	if (!item.tiers.empty()) {
		drawTiered(style, item, cell, content);
		return;
	}
	const std::optional<int>& price = item.price;

	drawIcon(style.iconBox(cell), item.contentId, content);
	text_.drawLeftWrapped(item.label,
			      style.labelBox(cell, price.has_value()),
			      style.textColor, style.textSize, 0);
	if (price.has_value()) {
		drawPrice(style, style.coinBox(cell), style.priceBox(cell),
			  *price);
	}
}

void MarketPopupPainter::drawTiered(const world::MarketPopupStyle& style,
				    const world::MarketItem& item,
				    const world::PixelBox& cell,
				    const world::ContentCatalog& content) {
	// Encabezado: ícono y nombre del producto, como un ítem común.
	const world::PixelBox header = style.headerBox(cell);
	drawIcon(style.iconBox(header), item.contentId, content);
	text_.drawLeftWrapped(item.label, style.labelBox(header, false),
			      style.textColor, style.textSize, 0);

	// Debajo, una línea por calidad: estrella + nombre y, a la misma
	// altura, el monto.
	for (std::size_t i = 0; i < item.tiers.size(); ++i) {
		const world::MarketTier& tier = item.tiers[i];
		const world::PixelBox line = style.tierLineBox(cell, i);
		if (tier.star.has_value()) {
			const world::PixelBox star = style.starBox(line);
			sheets_.sheet(style.star.sheetImage,
				      core::assetPath(style.star.sheetImage),
				      style.star.cellWidth,
				      style.star.cellHeight)
				.draw(tier.star->col, tier.star->row,
				      SDL2pp::Rect(star.x, star.y, star.width,
						   star.height));
		}
		text_.drawLeftWrapped(tier.label, style.tierLabelBox(line),
				      style.textColor, style.textSize, 0);
		if (tier.price.has_value()) {
			drawPrice(style, style.tierCoinBox(line),
				  style.tierPriceBox(line), *tier.price);
		}
	}
}

void MarketPopupPainter::drawIcon(const world::PixelBox& iconBox,
				  const std::string& contentId,
				  const world::ContentCatalog& content) {
	const world::SpriteVisual* visual = content.find(contentId);
	if (visual == nullptr) {
		return;
	}
	// El ícono entra en su caja sin deformarse.
	const world::PixelBox box = visual->fitIn(iconBox);
	SpriteSheet& sheet = sheets_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	sheet.drawBlock(visual->col, visual->row, visual->cols, visual->rows,
			SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void MarketPopupPainter::drawPrice(const world::MarketPopupStyle& style,
				   const world::PixelBox& coinBox,
				   const world::PixelBox& priceBox,
				   const int price) {
	const world::SpriteVisual& coin = style.coin;
	sheets_.sheet(coin.sheetImage, core::assetPath(coin.sheetImage),
		      coin.cellWidth, coin.cellHeight)
		.draw(coin.col, coin.row,
		      SDL2pp::Rect(coinBox.x, coinBox.y, coinBox.width,
				   coinBox.height));
	text_.drawLeftWrapped(std::to_string(price), priceBox,
			      style.priceColor, style.textSize, 0);
}

}
