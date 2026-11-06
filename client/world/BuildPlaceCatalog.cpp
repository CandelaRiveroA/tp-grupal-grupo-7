#include "world/BuildPlaceCatalog.h"

namespace world {

void BuildPlaceCatalog::setState(const BuildPlaceState state,
				 BuildPlaceVisual visual) {
	states_[state] = visual;
}

const BuildPlaceVisual* BuildPlaceCatalog::find(
	const BuildPlaceState state) const {
	const auto it = states_.find(state);
	return it == states_.end() ? nullptr : &it->second;
}

}
