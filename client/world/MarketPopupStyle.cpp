#include "world/MarketPopupStyle.h"

#include <algorithm>
#include <numeric>

namespace world {

int MarketPopupStyle::cellHeightOf(const MarketItem& item) const {
	if (item.tiers.empty()) {
		return rowHeight;
	}
	return headerHeight + static_cast<int>(item.tiers.size()) * tierHeight;
}

std::vector<int> MarketPopupStyle::rowHeightsOf(
	const MarketSection& section) const {
	std::vector<int> heights;
	for (std::size_t i = 0; i < section.items.size(); ++i) {
		const std::size_t row = i / static_cast<std::size_t>(columns);
		if (row >= heights.size()) {
			heights.push_back(0);
		}
		heights[row] =
			std::max(heights[row], cellHeightOf(section.items[i]));
	}
	return heights;
}

int MarketPopupStyle::sectionHeight(const MarketSection& section) const {
	const std::vector<int> heights = rowHeightsOf(section);
	return titleHeight + std::accumulate(heights.begin(), heights.end(), 0);
}

int MarketPopupStyle::sectionTop(const PixelBox& panel,
				 const std::vector<MarketSection>& sections,
				 const std::size_t section) const {
	// Las filas del panel son de a un slot: si sobra alto, el contenido se
	// reparte parejo arriba y abajo.
	const int slack = std::max(0, panel.height - contentHeight(sections));
	int top = panel.y + slack / 2 + inset;
	for (std::size_t i = 0; i < section; ++i) {
		top += sectionHeight(sections[i]) + sectionGap;
	}
	return top;
}

int MarketPopupStyle::contentHeight(
	const std::vector<MarketSection>& sections) const {
	int height = 2 * inset;
	for (std::size_t i = 0; i < sections.size(); ++i) {
		height += sectionHeight(sections[i]);
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
	const MarketSection& current = sections[section];
	const int width = std::max(1, (panel.width - 2 * inset) / columns);
	const std::size_t column = item % static_cast<std::size_t>(columns);
	const std::size_t row = item / static_cast<std::size_t>(columns);
	const std::vector<int> heights = rowHeightsOf(current);
	const int rowOffset = std::accumulate(
		heights.begin(), heights.begin() + static_cast<long>(row), 0);
	return PixelBox{panel.x + inset + static_cast<int>(column) * width,
			sectionTop(panel, sections, section) + titleHeight +
				rowOffset,
			width, cellHeightOf(current.items[item])};
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

PixelBox MarketPopupStyle::headerBox(const PixelBox& cell) const {
	return PixelBox{cell.x, cell.y, cell.width, headerHeight};
}

PixelBox MarketPopupStyle::tierLineBox(const PixelBox& cell,
				       const std::size_t index) const {
	return PixelBox{cell.x,
			cell.y + headerHeight +
				static_cast<int>(index) * tierHeight,
			cell.width, tierHeight};
}

PixelBox MarketPopupStyle::starBox(const PixelBox& line) const {
	const int width = star.displayWidth();
	const int height = star.displayHeight();
	return PixelBox{line.x, line.y + (line.height - height) / 2, width,
			height};
}

PixelBox MarketPopupStyle::tierTextBox(const PixelBox& line, const int left,
				       const int width) const {
	// drawLeftWrapped arranca arriba de la caja: se baja para que el
	// texto quede centrado en la línea.
	const int offset = std::max(0, (line.height - textSize) / 2);
	return PixelBox{line.x + left, line.y + offset, std::max(1, width),
			std::max(1, line.height - offset)};
}

PixelBox MarketPopupStyle::tierLabelBox(const PixelBox& line) const {
	const int left = star.displayWidth() + starGap;
	return tierTextBox(line, left, tierPriceLeft - left);
}

PixelBox MarketPopupStyle::tierCoinBox(const PixelBox& line) const {
	const int width = coin.displayWidth();
	const int height = coin.displayHeight();
	return PixelBox{line.x + tierPriceLeft,
			line.y + (line.height - height) / 2, width, height};
}

PixelBox MarketPopupStyle::tierPriceBox(const PixelBox& line) const {
	const int left = tierPriceLeft + coin.displayWidth() + coinGap;
	return tierTextBox(line, left, line.width - left);
}

}
