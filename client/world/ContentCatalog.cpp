#include "world/ContentCatalog.h"

namespace world {

const SpriteVisual* ContentCatalog::find(const std::string& contentId) const {
	const auto it = visuals.find(contentId);
	return it == visuals.end() ? nullptr : &it->second;
}

}  // namespace world
