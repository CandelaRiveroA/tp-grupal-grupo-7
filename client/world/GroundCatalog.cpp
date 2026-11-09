#include "world/GroundCatalog.h"

namespace world {

void GroundCatalog::setConsequence(const GroundConsequence consequence,
				   GroundVisual visual) {
	visuals_[consequence] = std::move(visual);
}

const GroundVisual* GroundCatalog::find(
	const GroundConsequence consequence) const {
	const auto it = visuals_.find(consequence);
	return it == visuals_.end() ? nullptr : &it->second;
}

const SpriteVisual* GroundCatalog::pieceFor(
	const GroundConsequence consequence, const std::uint8_t mask,
	const GridCell& cell) const {
	const GroundVisual* visual = find(consequence);
	if (visual == nullptr) {
		return nullptr;
	}
	const auto it = visual->pieces.find(mask);
	if (it == visual->pieces.end() || it->second.empty()) {
		// Sin pieza para esta máscara: el sprite simple.
		return &visual->sprite;
	}
	const std::vector<SpriteVisual>& variants = it->second;
	int variant = (cell.column + cell.row) % static_cast<int>(variants.size());
	if (variant < 0) {
		variant += static_cast<int>(variants.size());
	}
	return &variants[static_cast<std::size_t>(variant)];
}

}
