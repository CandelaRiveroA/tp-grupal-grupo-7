#include "world/CommunityPopupStyle.h"

#include <algorithm>

namespace world {

CommunityPopupLayout::CommunityPopupLayout(const int screenW, const int screenH,
					   const CommunityPopupStyle& style)
	: style_(style)
	, modal_(screenW, screenH, style_.modal,
		 style_.topBar.superTitleTop + style_.topBar.superTitleHeight +
			 style_.header.gap + style_.header.height +
			 style_.card.contentGap,
		 style_.footer.height + style_.modal.framePad) {
	const PixelBox& panel = modal_.panel();
	const int framePad = style_.modal.framePad;

	const int topLeft = panel.x + framePad;
	const int counterX =
		panel.x + panel.width - framePad - style_.topBar.pageCounterWidth;
	superTitleBox_ = PixelBox{topLeft, panel.y + style_.topBar.superTitleTop,
				  std::max(1, counterX - topLeft),
				  style_.topBar.superTitleHeight};
	pageCounterBox_ =
		PixelBox{counterX, panel.y + style_.topBar.superTitleTop,
			 style_.topBar.pageCounterWidth,
			 style_.topBar.superTitleHeight};

	headerBox_ = PixelBox{
		topLeft,
		panel.y + style_.topBar.superTitleTop +
			style_.topBar.superTitleHeight + style_.header.gap,
		std::max(1, panel.width - 2 * framePad), style_.header.height};

	const int footerTop =
		panel.y + panel.height - framePad - style_.footer.height;
	const int donateRight =
		panel.x + panel.width - framePad - style_.footer.margin;
	donateButtonBox_ = PixelBox{
		donateRight - style_.footer.donateButtonWidth,
		footerTop +
			(style_.footer.height - style_.footer.donateButtonHeight) /
				2,
		style_.footer.donateButtonWidth,
		style_.footer.donateButtonHeight};

	helpPopupBox_ = PixelBox{
		panel.x + (panel.width - style_.help.popupWidth) / 2,
		panel.y + (panel.height - style_.help.popupHeight) / 2,
		style_.help.popupWidth, style_.help.popupHeight};
	const int helpInnerW =
		std::max(1, helpPopupBox_.width - 2 * style_.help.pad);
	helpTitleBox_ = PixelBox{helpPopupBox_.x + style_.help.pad,
				 helpPopupBox_.y + style_.help.pad, helpInnerW,
				 style_.help.titleHeight};
	const int bodyTop = helpPopupBox_.y + style_.help.pad +
			    style_.help.titleHeight + style_.help.titleGap;
	const int bodyBottom = helpPopupBox_.y + helpPopupBox_.height -
			       style_.help.pad - style_.help.hintHeight;
	helpBodyBox_ = PixelBox{helpPopupBox_.x + style_.help.pad, bodyTop,
				helpInnerW, std::max(1, bodyBottom - bodyTop)};
	helpHintBox_ = PixelBox{helpPopupBox_.x + style_.help.pad, bodyBottom,
				helpInnerW, style_.help.hintHeight};
}

const PagedModalLayout& CommunityPopupLayout::modal() const { return modal_; }

const PixelBox& CommunityPopupLayout::superTitleBox() const {
	return superTitleBox_;
}

const PixelBox& CommunityPopupLayout::pageCounterBox() const {
	return pageCounterBox_;
}

const PixelBox& CommunityPopupLayout::headerBox() const { return headerBox_; }

const PixelBox& CommunityPopupLayout::donateButtonBox() const {
	return donateButtonBox_;
}

const PixelBox& CommunityPopupLayout::helpPopupBox() const {
	return helpPopupBox_;
}

const PixelBox& CommunityPopupLayout::helpTitleBox() const {
	return helpTitleBox_;
}

const PixelBox& CommunityPopupLayout::helpBodyBox() const {
	return helpBodyBox_;
}

const PixelBox& CommunityPopupLayout::helpHintBox() const {
	return helpHintBox_;
}

PixelBox CommunityPopupLayout::helpButtonBox(const int titleWidth) const {
	return PixelBox{
		headerBox_.x + titleWidth + style_.help.buttonGap,
		headerBox_.y + (headerBox_.height - style_.help.buttonHeight) / 2,
		style_.help.buttonWidth, style_.help.buttonHeight};
}

PixelBox CommunityPopupLayout::headerStatusBox(const PixelBox& helpBtn) const {
	const int left =
		(helpBtn.x - headerBox_.x) + helpBtn.width + style_.header.statusGap;
	return PixelBox{headerBox_.x + left, headerBox_.y,
			std::max(1, headerBox_.width - left),
			headerBox_.height};
}

PixelBox CommunityPopupLayout::headerBadgeBox(const PixelBox& helpBtn) const {
	const int width = style_.sprites.setDone.displayWidth();
	const int height = style_.sprites.setDone.displayHeight();
	return PixelBox{helpBtn.x + helpBtn.width + style_.header.statusGap,
			headerBox_.y + (headerBox_.height - height) / 2, width,
			height};
}

PixelBox CommunityPopupLayout::pageDotBox(const std::size_t pageCount,
					  const std::size_t index,
					  const bool hasDonateButton) const {
	const PixelBox& panel = modal_.panel();
	const int framePad = style_.modal.framePad;
	const int count = static_cast<int>(pageCount);
	const int dotSize = style_.footer.dotSize;
	const int totalW =
		count > 0 ? count * dotSize + (count - 1) * style_.footer.dotGap
			  : 0;
	const int footerTop =
		panel.y + panel.height - framePad - style_.footer.height;
	const int areaLeft = panel.x + framePad;
	const int areaRight = hasDonateButton
				      ? donateButtonBox_.x - style_.footer.donateGap
				      : panel.x + panel.width - framePad;
	const int startX =
		areaLeft + std::max(0, areaRight - areaLeft - totalW) / 2;
	return PixelBox{
		startX + static_cast<int>(index) * (dotSize + style_.footer.dotGap),
		footerTop + (style_.footer.height - dotSize) / 2, dotSize,
		dotSize};
}

PixelBox CommunityPopupLayout::cardBox(const std::size_t rowCount,
				       const std::size_t rowIndex) const {
	const PixelBox& area = modal_.innerContentRect();
	const int count = static_cast<int>(rowCount);
	const int totalH =
		count > 0 ? count * style_.card.height +
				    (count - 1) * style_.card.gap
			  : 0;
	const int slack = std::max(0, area.height - totalH);
	const int top = area.y + slack / 2 +
			static_cast<int>(rowIndex) *
				(style_.card.height + style_.card.gap);
	return PixelBox{area.x, top, area.width, style_.card.height};
}

PixelBox CommunityPopupLayout::spriteBox(const PixelBox& card,
					 const bool isCreature,
					 const int overrideSize,
					 const int offsetY) const {
	const int defaultSize = isCreature ? style_.card.creatureSpriteSize
					   : style_.card.spriteSize;
	const int size = overrideSize > 0 ? overrideSize : defaultSize;
	const int centerX =
		card.x + style_.card.padX + style_.card.iconColumnWidth / 2;
	return PixelBox{centerX - size / 2,
			card.y + (card.height - size) / 2 + offsetY, size,
			size};
}

PixelBox CommunityPopupLayout::qualitySlotBox(const PixelBox& card,
					      const std::size_t index) const {
	const int slotSize = style_.card.qualitySlotSize;
	const int x = card.x + style_.card.padX +
		      static_cast<int>(index) *
			      (slotSize + style_.card.qualitySlotGap);
	return PixelBox{x, card.y + (card.height - slotSize) / 2, slotSize,
			slotSize};
}

PixelBox CommunityPopupLayout::qualitySpriteBox(const PixelBox& slot) const {
	const PixelBox inner = slot.inset(style_.card.qualitySlotBorder +
					  style_.card.qualitySlotPad);
	return PixelBox{inner.x, inner.y, std::max(1, inner.width),
			std::max(1, inner.height)};
}

PixelBox CommunityPopupLayout::checkboxBox(const PixelBox& card) const {
	const int width = style_.sprites.checkboxEmpty.displayWidth();
	const int height = style_.sprites.checkboxEmpty.displayHeight();
	return PixelBox{card.x + card.width - style_.card.padX - width,
			card.y + (card.height - height) / 2, width, height};
}

PixelBox CommunityPopupLayout::amountBox(const PixelBox& card,
					 const int textWidth) const {
	const int width = std::min(textWidth, style_.card.amountWidth);
	const int right = cardRightLimitX(card);
	return PixelBox{right - width,
			card.y + style_.card.border + style_.card.padY,
			std::max(1, width), topHalfHeight(card)};
}

PixelBox CommunityPopupLayout::labelBox(const PixelBox& card,
					const std::size_t qualityCount) const {
	const int left = cardLeftX(card, qualityCount);
	const int right = cardRightLimitX(card) - style_.card.amountWidth;
	return PixelBox{left, card.y + style_.card.border + style_.card.padY,
			std::max(1, right - left), topHalfHeight(card)};
}

PixelBox CommunityPopupLayout::progressTrackBox(
	const PixelBox& card, const std::size_t qualityCount) const {
	const int left = cardLeftX(card, qualityCount);
	const int right = cardRightLimitX(card);
	const int top = card.y + style_.card.border + style_.card.padY +
			topHalfHeight(card) + style_.card.barTopGap;
	return PixelBox{left, top, std::max(1, right - left),
			style_.card.barHeight};
}

PixelBox CommunityPopupLayout::progressFillBox(const PixelBox& track,
					       const int current,
					       const int total,
					       const bool done) const {
	const PixelBox inner = track.inset(style_.card.barBorder);
	if (inner.width <= 0 || inner.height <= 0) {
		return PixelBox{inner.x, inner.y, 0, 0};
	}
	if (done) {
		return inner;
	}
	if (total <= 0 || current <= 0) {
		return PixelBox{inner.x, inner.y, 0, inner.height};
	}
	const int clamped = std::min(current, total);
	const int fillW = static_cast<int>(
		(static_cast<long long>(inner.width) * clamped) / total);
	return PixelBox{inner.x, inner.y, std::max(0, fillW), inner.height};
}

bool CommunityPopupLayout::hitsHelpButton(const int titleWidth, const int x,
					  const int y) const {
	return helpButtonBox(titleWidth).contains(x, y);
}

bool CommunityPopupLayout::hitsDonateButton(const bool donatable, const int x,
					    const int y) const {
	return donatable && donateButtonBox_.contains(x, y);
}

int CommunityPopupLayout::cardLeftX(const PixelBox& card,
				    const std::size_t qualityCount) const {
	int columnW = style_.card.iconColumnWidth;
	if (qualityCount > 0) {
		const int count = static_cast<int>(qualityCount);
		const int slotsW =
			count * style_.card.qualitySlotSize +
			(count - 1) * style_.card.qualitySlotGap;
		columnW = std::max(columnW, slotsW);
	}
	return card.x + style_.card.padX + columnW + style_.card.spriteGap;
}

int CommunityPopupLayout::cardRightLimitX(const PixelBox& card) const {
	return checkboxBox(card).x - style_.card.checkboxGap;
}

int CommunityPopupLayout::topHalfHeight(const PixelBox& card) const {
	return std::max(1, card.height -
				   2 * (style_.card.border + style_.card.padY) -
				   style_.card.barHeight -
				   style_.card.barTopGap);
}

}  // namespace world
