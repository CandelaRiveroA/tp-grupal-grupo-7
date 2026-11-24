#include "sdl/render/CommunityPopupPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

CommunityPopupPainter::CommunityPopupPainter(SdlContext& context,
					     TextRenderer& text)
	: text_(text), sheets_(context) {}

void CommunityPopupPainter::draw(
	const world::CommunityPopupStyle& style, const std::string& title,
	const std::vector<world::CommunitySection>& sections,
	const world::PixelBox& panel) {
	text_.draw(title, style.titleBox(panel, sections), style.titleColor,
		   style.titleSize, true);
	for (std::size_t s = 0; s < sections.size(); ++s) {
		drawHeader(style, sections[s],
			   style.headerBox(panel, sections, s));
		for (std::size_t r = 0; r < sections[s].rows.size(); ++r) {
			drawRow(style, sections[s].rows[r],
				style.rowBox(panel, sections, s, r));
		}
	}
}

void CommunityPopupPainter::drawHeader(
	const world::CommunityPopupStyle& style,
	const world::CommunitySection& section, const world::PixelBox& header) {
	text_.drawLeft(section.title, header, style.headerColor,
		       style.headerSize, true);
	const int titleWidth =
		text_.textWidth(section.title, style.headerSize, true);
	if (section.done) {
		drawVisual(style.setDone, style.badgeBox(header, titleWidth));
		return;
	}
	text_.drawLeft(section.status, style.statusBox(header, titleWidth),
		       style.textColor, style.textSize);
}

void CommunityPopupPainter::drawRow(const world::CommunityPopupStyle& style,
				    const world::CommunityRow& row,
				    const world::PixelBox& box) {
	drawVisual(row.done ? style.checkboxDone : style.checkboxEmpty,
		   style.checkboxBox(box));
	text_.drawLeft(row.label, style.labelBox(box), style.textColor,
		       style.textSize);
	if (!row.amount.empty()) {
		const int width = text_.textWidth(row.amount, style.textSize);
		text_.drawLeft(row.amount, style.amountBox(box, width),
			       style.amountColor, style.textSize);
	}
}

void CommunityPopupPainter::drawVisual(const world::SpriteVisual& visual,
				       const world::PixelBox& box) {
	sheets_.sheet(visual.sheetImage, core::assetPath(visual.sheetImage),
		      visual.cellWidth, visual.cellHeight)
		.drawBlock(visual.col, visual.row, visual.cols, visual.rows,
			   SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

}
