#pragma once

#include <cstddef>
#include <string>

#include "world/Color.h"
#include "world/PagedModal.h"
#include "world/PixelBox.h"
#include "world/SpriteVisual.h"

namespace world {

// Configuración visual del botón "?" y su mini-popup de ayuda.
struct HelpPopupStyle {
	std::string buttonSheet;
	std::string buttonFallback;
	int buttonWidth = 0;
	int buttonHeight = 0;
	int buttonGap = 0;
	int popupWidth = 0;
	int popupHeight = 0;
	int pad = 0;
	int border = 0;
	int titleHeight = 0;
	int titleSize = 0;
	int titleGap = 0;
	int textSize = 0;
	int lineGap = 0;
	int hintHeight = 0;
	int hintSize = 0;
	Color backdrop{};
};

struct CommunityTopBarStyle {
	int superTitleTop = 0;
	int superTitleHeight = 0;
	int superTitleSize = 0;
	int pageCounterWidth = 0;
	int pageSize = 0;
};

struct CommunityHeaderStyle {
	int gap = 0;
	int height = 0;
	int titleSize = 0;
	int statusSize = 0;
	int statusGap = 0;
};

struct CommunityCardStyle {
	int contentGap = 0;
	int height = 0;
	int gap = 0;
	int padX = 0;
	int padY = 0;
	int border = 0;
	int iconColumnWidth = 0;
	int spriteSize = 0;
	int creatureSpriteSize = 0;
	int spriteGap = 0;
	int qualitySlotSize = 0;
	int qualitySlotGap = 0;
	int qualitySlotPad = 0;
	int qualitySlotBorder = 0;
	int barHeight = 0;
	int barBorder = 0;
	int barTopGap = 0;
	int amountWidth = 0;
	int checkboxGap = 0;
	int labelSize = 0;
	int amountSize = 0;
};

struct CommunityFooterStyle {
	int height = 0;
	int margin = 0;
	int donateGap = 0;
	int dotSize = 0;
	int dotGap = 0;
	int dotBorder = 0;
	int donateTextSize = 0;
	int donateButtonWidth = 0;
	int donateButtonHeight = 0;
	std::string donateButtonSheet;
};

struct CommunityPopupColors {
	Color inkMuted{};
	Color cardBg{};
	Color cardDoneBg{};
	Color spriteSlotBg{};
	Color barTrack{};
	Color barFill{};
	Color amountColor{};
	Color donateTextColor{};
};

struct CommunityPopupSprites {
	SpriteVisual checkboxEmpty;
	SpriteVisual checkboxDone;
	SpriteVisual setDone;
	SpriteVisual coin;
};

// Datos visuales puros del tablón del Centro Comunitario (sin SDL ni
// geometría). La geometría se calcula aparte en CommunityPopupLayout.
struct CommunityPopupStyle {
	PagedModalSpec modal;
	CommunityTopBarStyle topBar;
	CommunityHeaderStyle header;
	CommunityCardStyle card;
	CommunityFooterStyle footer;
	CommunityPopupColors colors;
	HelpPopupStyle help;
	CommunityPopupSprites sprites;
};

// Geometría pura del tablón paginado del Centro Comunitario (espejo de
// WorldMapLayout). Envuelve PagedModalLayout y precalcula las regiones fijas
// de barra superior, encabezado, ayuda y botón Donar sin recálculos.
class CommunityPopupLayout {
public:
	CommunityPopupLayout(int screenW, int screenH,
			     const CommunityPopupStyle& style);

	const PagedModalLayout& modal() const;
	const PixelBox& superTitleBox() const;
	const PixelBox& pageCounterBox() const;
	const PixelBox& headerBox() const;
	const PixelBox& donateButtonBox() const;
	const PixelBox& helpPopupBox() const;
	const PixelBox& helpTitleBox() const;
	const PixelBox& helpBodyBox() const;
	const PixelBox& helpHintBox() const;

	PixelBox helpButtonBox(int titleWidth) const;
	PixelBox headerStatusBox(const PixelBox& helpBtn) const;
	PixelBox headerBadgeBox(const PixelBox& helpBtn) const;

	PixelBox pageDotBox(std::size_t pageCount, std::size_t index,
			    bool hasDonateButton) const;

	PixelBox cardBox(std::size_t rowCount, std::size_t rowIndex) const;
	PixelBox spriteBox(const PixelBox& card, bool isCreature,
			   int overrideSize = 0, int offsetY = 0) const;
	PixelBox qualitySlotBox(const PixelBox& card, std::size_t index) const;
	PixelBox qualitySpriteBox(const PixelBox& slot) const;
	PixelBox checkboxBox(const PixelBox& card) const;
	PixelBox amountBox(const PixelBox& card, int textWidth) const;
	PixelBox labelBox(const PixelBox& card, std::size_t qualityCount) const;
	PixelBox progressTrackBox(const PixelBox& card,
				  std::size_t qualityCount) const;
	PixelBox progressFillBox(const PixelBox& track, int current, int total,
				 bool done) const;

	bool hitsHelpButton(int titleWidth, int x, int y) const;
	bool hitsDonateButton(bool donatable, int x, int y) const;

private:
	int cardLeftX(const PixelBox& card, std::size_t qualityCount) const;
	int cardRightLimitX(const PixelBox& card) const;
	int topHalfHeight(const PixelBox& card) const;

	CommunityPopupStyle style_;
	PagedModalLayout modal_;
	PixelBox superTitleBox_{};
	PixelBox pageCounterBox_{};
	PixelBox headerBox_{};
	PixelBox donateButtonBox_{};
	PixelBox helpPopupBox_{};
	PixelBox helpTitleBox_{};
	PixelBox helpBodyBox_{};
	PixelBox helpHintBox_{};
};

}  // namespace world
