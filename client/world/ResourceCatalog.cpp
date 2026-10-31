#include "world/ResourceCatalog.h"

namespace world {

bool ResourceCatalog::has(ResourceKind kind, ResourceState state) const {
	const auto kindIt = visuals.find(kind);
	if (kindIt == visuals.end()) {
		return false;
	}
	return kindIt->second.find(state) != kindIt->second.end();
}

const ResourceVisual& ResourceCatalog::visualFor(ResourceKind kind,
						 ResourceState state) const {
	return visuals.at(kind).at(state);
}

}
