#include "world/ContentCatalog.h"

namespace world {

bool ContentCatalog::has(const std::string& contentId) const {
	return visuals.find(contentId) != visuals.end();
}

const ItemVisual& ContentCatalog::visualFor(
	const std::string& contentId) const {
	return visuals.at(contentId);
}

}  // namespace world
