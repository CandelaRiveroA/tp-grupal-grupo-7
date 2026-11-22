#include "world/MarketPopupStyle.h"

#include <algorithm>

namespace world {

int MarketPopupStyle::rowsOf(const MarketSection& section) const {
	const int count = static_cast<int>(section.items.size());
	return (count + columns - 1) / columns;
}

int MarketPopupStyle::sectionTop(const PixelBox& panel,
				 const std::vector<MarketSection>& sections,
				 const std::size_t section) const {
	// Las filas del panel son de a un slot: si sobra alto, el contenido se
	// reparte parejo arriba y abajo.
	const int slack = std::max(0, panel.height - contentHeight(sections));
	int top = panel.y + slack / 2 + inset;
	for (std::size_t i = 0; i < section; ++i) {
		top += titleHeight + rowsOf(sections[i]) * rowHeight +
		       sectionGap;
	}
	return top;
}

int MarketPopupStyle::contentHeight(
	const std::vector<MarketSection>& sections) const {
	int height = 2 * inset;
	for (std::size_t i = 0; i < sections.size(); ++i) {
		height += titleHeight + rowsOf(sections[i]) * rowHeight;
		if (i > 0) {
			height += sectionGap;
		}
	}
	return height;
}

PixelBox MarketPopupStyle::titleBox(
	const PixelBox& panel, const std::vector<MarketSection>& sections,
	const std::size_t section) const {
	return PixelBox{panel.x + inset, sectionTop(panel, sections, section),
			std::max(1, panel.width - 2 * inset), titleHeight};
}

PixelBox MarketPopupStyle::cellBox(
	const PixelBox& panel, const std::vector<MarketSection>& sections,
	const std::size_t section, const std::size_t item) const {
	const int width = std::max(1, (panel.width - 2 * inset) / columns);
	const int column = static_cast<int>(item) % columns;
	const int row = static_cast<int>(item) / columns;
	return PixelBox{panel.x + inset + column * width,
			sectionTop(panel, sections, section) + titleHeight +
				row * rowHeight,
			width, rowHeight};
}

PixelBox MarketPopupStyle::iconBox(const PixelBox& cell) const {
	return PixelBox{cell.x, cell.y + (cell.height - iconSize) / 2,
			iconSize, iconSize};
}

PixelBox MarketPopupStyle::labelBox(const PixelBox& cell,
				    const bool withPrice) const {
	const int left = iconSize + iconGap;
	const int reserved = withPrice ? priceHeight : 0;
	const int top = iconBox(cell).y + labelOffsetY;
	return PixelBox{cell.x + left, top, std::max(1, cell.width - left),
			std::max(1, cell.y + cell.height - reserved - top)};
}

PixelBox MarketPopupStyle::coinBox(const PixelBox& cell) const {
	const int width = coin.displayWidth();
	const int height = coin.displayHeight();
	return PixelBox{cell.x + iconSize + iconGap,
			cell.y + cell.height - priceHeight +
				(priceHeight - height) / 2,
			width, height};
}

PixelBox MarketPopupStyle::priceBox(const PixelBox& cell) const {
	const int left = iconSize + iconGap + coin.displayWidth() + coinGap;
	return PixelBox{cell.x + left, cell.y + cell.height - priceHeight,
			std::max(1, cell.width - left), priceHeight};
}

}
