#include "game/CommunityPopupState.h"

namespace game {

CommunityPopupState::CommunityPopupState(
	const world::CommunityCatalog& catalog,
	const world::InventoryLayout& inventory)
	: catalog_(catalog), inventory_(inventory) {}

void CommunityPopupState::toggle() { open_ = !open_; }

void CommunityPopupState::close() { open_ = false; }

bool CommunityPopupState::isOpen() const { return open_; }

world::GridSpan CommunityPopupState::panelGrid(
	const std::vector<world::CommunitySection>& sections) const {
	const world::CommunityPopupStyle& style = catalog_.style();
	return world::GridSpan{
		style.panelColumns,
		inventory_.rowsFitting(style.contentHeight(sections))};
}

world::PixelBox CommunityPopupState::panel(
	const std::vector<world::CommunitySection>& sections,
	const int windowWidth, const int windowHeight) const {
	const world::GridSpan grid = panelGrid(sections);
	return inventory_.centeredBox(windowWidth, windowHeight, grid.columns,
				      grid.rows);
}

}
