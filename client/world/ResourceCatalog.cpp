#include "world/ResourceCatalog.h"

namespace world {

const ResourceVisual* ResourceCatalog::find(ResourceKind kind,
					    ResourceState state) const {
	const auto kindIt = visuals.find(kind);
	if (kindIt == visuals.end()) {
		return nullptr;
	}
	const auto stateIt = kindIt->second.find(state);
	return stateIt == kindIt->second.end() ? nullptr : &stateIt->second;
}

}
