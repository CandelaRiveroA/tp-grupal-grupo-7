#include "world/ResourceCatalog.h"

namespace world {

const ResourceVisual* ResourceCatalog::find(const std::string& contentId,
					    ResourceState state) const {
	const auto resourceIt = visuals.find(contentId);
	if (resourceIt == visuals.end()) {
		return nullptr;
	}
	const auto stateIt = resourceIt->second.find(state);
	return stateIt == resourceIt->second.end() ? nullptr : &stateIt->second;
}

}
