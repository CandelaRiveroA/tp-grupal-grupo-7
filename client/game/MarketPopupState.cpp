#include "game/MarketPopupState.h"

#include <utility>

namespace game {

MarketPopupState::MarketPopupState(const world::MarketCatalog& catalog,
				   const world::InventoryLayout& inventory)
	: catalog_(catalog), inventory_(inventory), resolver_(catalog) {}

void MarketPopupState::open(const world::MarketOffer& offer) {
	std::vector<world::MarketSection> sections =
		resolver_.sectionsFrom(offer);
	if (sections.empty()) {
		return;
	}
	const world::MarketPopupStyle& style = catalog_.style();
	panelGrid_ = world::GridSpan{
		style.panelColumns,
		inventory_.rowsFitting(style.contentHeight(sections))};
	sections_ = std::move(sections);
	open_ = true;
}

void MarketPopupState::close() {
	open_ = false;
	sections_.clear();
}

bool MarketPopupState::isOpen() const { return open_; }

const std::vector<world::MarketSection>& MarketPopupState::sections() const {
	return sections_;
}

world::GridSpan MarketPopupState::panelGrid() const { return panelGrid_; }

world::PixelBox MarketPopupState::panel(const int windowWidth,
					const int windowHeight) const {
	return inventory_.centeredBox(windowWidth, windowHeight,
				      panelGrid_.columns, panelGrid_.rows);
}

}
