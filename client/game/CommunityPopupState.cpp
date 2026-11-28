#include "game/CommunityPopupState.h"

namespace game {

void CommunityPopupState::open() { open_ = true; }

void CommunityPopupState::close() {
	open_ = false;
	helpOpen_ = false;
}

bool CommunityPopupState::isOpen() const { return open_; }

void CommunityPopupState::openHelp() { helpOpen_ = true; }

void CommunityPopupState::closeHelp() { helpOpen_ = false; }

bool CommunityPopupState::isHelpOpen() const { return helpOpen_; }

std::size_t CommunityPopupState::pageIndex(const std::size_t pageCount) const {
	if (pageCount == 0) {
		return 0;
	}
	return pageIndex_ % pageCount;
}

void CommunityPopupState::nextPage(const std::size_t pageCount) {
	if (pageCount == 0) {
		return;
	}
	helpOpen_ = false;
	pageIndex_ = (pageIndex_ + 1) % pageCount;
}

void CommunityPopupState::prevPage(const std::size_t pageCount) {
	if (pageCount == 0) {
		return;
	}
	helpOpen_ = false;
	pageIndex_ = (pageIndex_ % pageCount + pageCount - 1) % pageCount;
}

}  // namespace game
