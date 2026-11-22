#include "world/NpcVisuals.h"

#include <algorithm>

namespace world {

PixelRect NpcBody::box(const int cellPixelSize, const GridCell& where) const {
	const int width = drawn.width > 0 ? drawn.width
					  : span.columns * cellPixelSize;
	const int height = drawn.height > 0 ? drawn.height
					    : span.rows * cellPixelSize;
	return PixelRect{where.column * cellPixelSize,
			 where.row * cellPixelSize, width, height};
}

std::vector<NpcBodyPiece> NpcBody::pieces(const int cellPixelSize,
					  const GridCell& where,
					  const int frame) const {
	std::vector<NpcBodyPiece> result;
	const bool animated = frame >= 0 &&
			      static_cast<std::size_t>(frame) < frames.size();
	const std::vector<GridCell>& source =
		animated ? frames[static_cast<std::size_t>(frame)] : cells;
	if (span.columns <= 0 || span.rows <= 0) {
		return result;
	}
	const PixelRect whole = box(cellPixelSize, where);
	const int pieceWidth = whole.width / span.columns;
	const int pieceHeight = whole.height / span.rows;
	if (pieceWidth <= 0 || pieceHeight <= 0) {
		return result;
	}
	for (int row = 0; row < span.rows; ++row) {
		for (int column = 0; column < span.columns; ++column) {
			const std::size_t index = static_cast<std::size_t>(
				row * span.columns + column);
			if (index >= source.size()) {
				return result;
			}
			result.push_back(NpcBodyPiece{
				source[index],
				PixelRect{whole.x + column * pieceWidth,
					  whole.y + row * pieceHeight,
					  pieceWidth, pieceHeight}});
		}
	}
	return result;
}

PixelBox NpcPortrait::fit(const PixelBox& box) const {
	const int sourceWidth = cellWidth * span.columns;
	const int sourceHeight = cellHeight * span.rows;
	if (sourceWidth <= 0 || sourceHeight <= 0 || box.width <= 0 ||
	    box.height <= 0) {
		return box;
	}
	const double scale = std::min(
		static_cast<double>(box.width) / sourceWidth,
		static_cast<double>(box.height) / sourceHeight);
	const int width = std::max(1, static_cast<int>(sourceWidth * scale));
	const int height = std::max(1, static_cast<int>(sourceHeight * scale));
	return PixelBox{box.x + (box.width - width) / 2,
			box.y + (box.height - height) / 2, width, height};
}

PixelBox NpcPopupStyle::areaBox(const PixelBox& panel,
				const NpcGridBox& area, const int inset) const {
	const int slotWidth = panel.width / grid.columns;
	const int slotHeight = panel.height / grid.rows;
	return PixelBox{
		panel.x + area.at.column * slotWidth + inset,
		panel.y + area.at.row * slotHeight + inset,
		std::max(1, area.span.columns * slotWidth - 2 * inset),
		std::max(1, area.span.rows * slotHeight - 2 * inset)};
}

PixelBox NpcPopupStyle::portraitBox(const PixelBox& panel) const {
	return areaBox(panel, portrait, portraitInset);
}

PixelBox NpcPopupStyle::placePortrait(const PixelBox& fitted) const {
	const int width = std::max(
		1, static_cast<int>(fitted.width * portraitScale + 0.5));
	const int height = std::max(
		1, static_cast<int>(fitted.height * portraitScale + 0.5));
	return PixelBox{fitted.x + portraitOffset.x,
			fitted.y - (height - fitted.height) / 2 + portraitOffset.y,
			width, height};
}

PixelBox NpcPopupStyle::textBox(const PixelBox& panel) const {
	PixelBox box = areaBox(panel, text, textInset);
	box.y += textOffset.y;
	box.height = std::max(1, box.height - textOffset.y);
	box.x += textOffset.x;
	box.width = std::max(1, box.width - textOffset.x);
	return box;
}

}
