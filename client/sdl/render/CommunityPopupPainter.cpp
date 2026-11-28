#include "sdl/render/CommunityPopupPainter.h"

#include "core/Constants.h"
#include "sdl/context/SdlConstants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/CharacterSprite.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TextRenderer.h"
#include "world/CharacterCatalog.h"
#include "world/ContentCatalog.h"
#include "world/CropCatalog.h"
#include "world/EntityAppearance.h"

namespace sdl {

CommunityPopupPainter::CommunityPopupPainter(
	SdlContext& context, InventoryHud& inventoryHud, TextRenderer& text,
	SheetCache& sheets, const world::ContentCatalog& content,
	const world::CropCatalog& crops,
	const world::CharacterCatalog& characters,
	const world::CommunityCatalog& catalog)
	: text_(text)
	, sheets_(sheets)
	, content_(content)
	, crops_(crops)
	, characters_(characters)
	, catalog_(catalog)
	, modalPainter_(context, inventoryHud, text, catalog.style().modal)
	, visualDrawers_{
		  {world::CommunityVisualKind::CropQualities,
		   &CommunityPopupPainter::drawCropQualitiesVisual},
		  {world::CommunityVisualKind::Creature,
		   &CommunityPopupPainter::drawCreatureVisual},
		  {world::CommunityVisualKind::Coin,
		   &CommunityPopupPainter::drawCoinVisual},
		  {world::CommunityVisualKind::ContentIcon,
		   &CommunityPopupPainter::drawContentIconVisual},
	  }
	, donateButtonTex_(context.tryLoadTexture(
		  core::assetPath(catalog.style().footer.donateButtonSheet)))
	, helpButtonTex_(context.tryLoadTexture(
		  core::assetPath(catalog.style().help.buttonSheet))) {}

bool CommunityPopupPainter::hitsHelpButton(
	const world::CommunityPopupLayout& layout, const std::string& setTitle,
	const int x, const int y) {
	const int titleWidth = text_.textWidth(
		setTitle, catalog_.style().header.titleSize, true);
	return layout.hitsHelpButton(titleWidth, x, y);
}

void CommunityPopupPainter::draw(
	const std::vector<world::CommunitySection>& sections,
	const std::size_t pageIndex, const bool helpOpen, const int iteration,
	const int screenW, const int screenH) {
	if (sections.empty()) {
		return;
	}
	const std::size_t activeIndex = pageIndex % sections.size();
	const world::CommunitySection& section = sections[activeIndex];
	const world::CommunityPopupLayout layout(screenW, screenH,
						 catalog_.style());

	modalPainter_.drawShell(layout.modal(), screenW, screenH);
	drawTopBar(layout, activeIndex, sections.size());
	drawSetHeader(layout, section);

	for (std::size_t r = 0; r < section.rows.size(); ++r) {
		drawCard(layout, section.rows[r],
			 layout.cardBox(section.rows.size(), r), iteration);
	}

	drawFooter(layout, section, activeIndex, sections.size());
	modalPainter_.drawArrows(layout.modal());

	if (helpOpen) {
		drawHelpPopup(layout, section, screenW, screenH);
	}
}

void CommunityPopupPainter::drawTopBar(
	const world::CommunityPopupLayout& layout, const std::size_t pageIndex,
	const std::size_t pageCount) {
	const world::CommunityPopupStyle& style = catalog_.style();
	text_.drawLeft(catalog_.title(), layout.superTitleBox(),
		       style.colors.inkMuted, style.topBar.superTitleSize);
	const std::string counter = std::to_string(pageIndex + 1) + " / " +
				    std::to_string(pageCount);
	text_.draw(counter, layout.pageCounterBox(), style.colors.inkMuted,
		   style.topBar.pageSize, true);
}

void CommunityPopupPainter::drawSetHeader(
	const world::CommunityPopupLayout& layout,
	const world::CommunitySection& section) {
	const world::CommunityPopupStyle& style = catalog_.style();
	text_.drawLeft(section.title, layout.headerBox(), style.modal.inkColor,
		       style.header.titleSize, true);
	const int titleWidth =
		text_.textWidth(section.title, style.header.titleSize, true);

	const world::PixelBox helpBtn = layout.helpButtonBox(titleWidth);
	if (helpButtonTex_.has_value()) {
		modalPainter_.drawTexture(*helpButtonTex_, helpBtn);
	} else {
		modalPainter_.drawBorderedBox(helpBtn, style.colors.cardBg,
					      style.modal.woodBorderColor,
					      style.card.border);
		text_.draw(style.help.buttonFallback, helpBtn,
			   style.modal.inkColor, style.help.textSize, false);
	}

	if (section.done) {
		drawVisual(style.sprites.setDone,
			   layout.headerBadgeBox(helpBtn));
		return;
	}
	text_.drawLeft(catalog_.status(), layout.headerStatusBox(helpBtn),
		       style.modal.inkColor, style.header.statusSize);
}

void CommunityPopupPainter::drawCard(const world::CommunityPopupLayout& layout,
				     const world::CommunityRow& row,
				     const world::PixelBox& card,
				     const int iteration) {
	const world::CommunityPopupStyle& style = catalog_.style();
	const world::Color& fill =
		row.done ? style.colors.cardDoneBg : style.colors.cardBg;
	modalPainter_.drawBorderedBox(card, fill, style.modal.woodBorderColor,
				      style.card.border);
	drawRowSprite(layout, row, card, iteration);

	const std::size_t qualityCount = row.visual.cropQualities.size();
	text_.drawLeft(row.label, layout.labelBox(card, qualityCount),
		       style.modal.inkColor, style.card.labelSize, false);

	const world::PixelBox track =
		layout.progressTrackBox(card, qualityCount);
	modalPainter_.drawBorderedBox(track, style.colors.barTrack,
				      style.modal.woodBorderColor,
				      style.card.barBorder);
	const world::PixelBox fillBox = layout.progressFillBox(
		track, row.current, row.total, row.done);
	if (fillBox.width > 0 && fillBox.height > 0) {
		modalPainter_.fillRect(fillBox, style.colors.barFill);
	}

	if (!row.amount.empty()) {
		const int width = text_.textWidth(
			row.amount, style.card.amountSize, false);
		text_.drawLeft(row.amount, layout.amountBox(card, width),
			       style.colors.amountColor,
			       style.card.amountSize, false);
	}
	drawVisual(row.done ? style.sprites.checkboxDone
			    : style.sprites.checkboxEmpty,
		   layout.checkboxBox(card));
}

void CommunityPopupPainter::drawRowSprite(
	const world::CommunityPopupLayout& layout,
	const world::CommunityRow& row, const world::PixelBox& card,
	const int iteration) {
	const auto it = visualDrawers_.find(row.visual.kind);
	if (it != visualDrawers_.end()) {
		(this->*(it->second))(layout, row, card, iteration);
	}
}

void CommunityPopupPainter::drawCropQualitiesVisual(
	const world::CommunityPopupLayout& layout,
	const world::CommunityRow& row, const world::PixelBox& card,
	const int /*iteration*/) {
	const world::CropDefinition* crop = crops_.find(row.visual.iconId);
	if (crop == nullptr) {
		return;
	}
	const world::CommunityPopupStyle& style = catalog_.style();
	const int harvestStage = crop->finalStage();
	for (std::size_t i = 0; i < row.visual.cropQualities.size(); ++i) {
		const world::PixelBox slot = layout.qualitySlotBox(card, i);
		modalPainter_.drawBorderedBox(slot, style.colors.spriteSlotBg,
					      style.modal.woodBorderColor,
					      style.card.qualitySlotBorder);
		const world::SpriteVisual* sprite = crop->spriteForStage(
			harvestStage, row.visual.cropQualities[i]);
		if (sprite != nullptr) {
			drawFittedVisual(*sprite,
					 layout.qualitySpriteBox(slot));
		}
	}
}

void CommunityPopupPainter::drawCreatureVisual(
	const world::CommunityPopupLayout& layout,
	const world::CommunityRow& row, const world::PixelBox& card,
	const int iteration) {
	if (!row.visual.creature.has_value()) {
		return;
	}
	drawCreature(*row.visual.creature,
		     layout.spriteBox(card, true, row.visual.spriteSize,
				      row.visual.spriteOffsetY),
		     iteration);
}

void CommunityPopupPainter::drawCoinVisual(
	const world::CommunityPopupLayout& layout,
	const world::CommunityRow& row, const world::PixelBox& card,
	const int /*iteration*/) {
	drawFittedVisual(catalog_.style().sprites.coin,
			 layout.spriteBox(card, false, row.visual.spriteSize,
					  row.visual.spriteOffsetY));
}

void CommunityPopupPainter::drawContentIconVisual(
	const world::CommunityPopupLayout& layout,
	const world::CommunityRow& row, const world::PixelBox& card,
	const int /*iteration*/) {
	const world::SpriteVisual* visual = content_.find(row.visual.iconId);
	if (visual == nullptr) {
		return;
	}
	drawFittedVisual(*visual,
			 layout.spriteBox(card, false, row.visual.spriteSize,
					  row.visual.spriteOffsetY));
}

void CommunityPopupPainter::drawCreature(const world::CreatureKind kind,
					 const world::PixelBox& box,
					 const int iteration) {
	const world::CreatureData* creature = characters_.find(kind);
	if (creature == nullptr) {
		return;
	}
	const world::AppearanceResolver resolver(*creature);
	const world::EntityAppearance appearance = resolver.appearanceFor(
		world::Direction::Front, world::ActionKind::Walk, iteration);
	if (appearance.pose == nullptr || appearance.pose->empty() ||
	    appearance.action == nullptr) {
		return;
	}

	const world::CreatureSheetSpec spec =
		characters_.sheetFor(kind, *appearance.action);
	const world::PixelBox fit = box.fitCentered(
		world::PixelSize{spec.drawnWidth, spec.drawnHeight});
	const SDL2pp::Rect drawBox(fit.x, fit.y, fit.width, fit.height);

	SpriteSheet& sheet =
		sheets_.sheet(spec.file, core::assetPath(spec.file),
			      spec.cellWidth, spec.cellHeight);
	CharacterSprite sprite(sheet, fit.width, fit.height, spec.frameCols,
			       spec.frameRows);
	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;
	sprite.draw(*appearance.pose, drawBox, flip);
}

void CommunityPopupPainter::drawVisual(const world::SpriteVisual& visual,
				       const world::PixelBox& box) {
	sheets_.sheet(visual.sheetImage, core::assetPath(visual.sheetImage),
		      visual.cellWidth, visual.cellHeight)
		.drawBlock(visual.col, visual.row, visual.cols, visual.rows,
			   SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void CommunityPopupPainter::drawFittedVisual(const world::SpriteVisual& visual,
					     const world::PixelBox& box) {
	drawVisual(visual, visual.fitIn(box));
}

void CommunityPopupPainter::drawFooter(
	const world::CommunityPopupLayout& layout,
	const world::CommunitySection& section, const std::size_t pageIndex,
	const std::size_t pageCount) {
	const world::CommunityPopupStyle& style = catalog_.style();
	for (std::size_t i = 0; i < pageCount; ++i) {
		const world::PixelBox dot =
			layout.pageDotBox(pageCount, i, section.donatable);
		const world::Color& fill =
			(i == pageIndex) ? style.modal.inkColor
					 : style.colors.barTrack;
		modalPainter_.drawBorderedBox(dot, fill,
					      style.modal.woodBorderColor,
					      style.footer.dotBorder);
	}
	if (!section.donatable) {
		return;
	}
	const world::PixelBox& btnBox = layout.donateButtonBox();
	if (donateButtonTex_.has_value()) {
		modalPainter_.drawTexture(*donateButtonTex_, btnBox);
	} else {
		modalPainter_.drawBorderedBox(btnBox, style.colors.cardBg,
					      style.modal.woodBorderColor,
					      style.card.border);
	}
	text_.draw(catalog_.donateLabel(), btnBox, style.colors.donateTextColor,
		   style.footer.donateTextSize, false);
}

void CommunityPopupPainter::drawHelpPopup(
	const world::CommunityPopupLayout& layout,
	const world::CommunitySection& section, const int screenW,
	const int screenH) {
	const world::CommunityPopupStyle& style = catalog_.style();
	modalPainter_.fillRect(world::PixelBox{0, 0, screenW, screenH},
			       style.help.backdrop);
	modalPainter_.drawBorderedBox(layout.helpPopupBox(),
				      style.colors.cardBg,
				      style.modal.woodBorderColor,
				      style.help.border);
	text_.draw(section.title, layout.helpTitleBox(), style.modal.inkColor,
		   style.help.titleSize, true);
	text_.drawLeftWrapped(section.helpText, layout.helpBodyBox(),
			      style.modal.inkColor, style.help.textSize,
			      style.help.lineGap);
	text_.draw(catalog_.helpCloseHint(), layout.helpHintBox(),
		   style.colors.inkMuted, style.help.hintSize, false);
}

}  // namespace sdl
