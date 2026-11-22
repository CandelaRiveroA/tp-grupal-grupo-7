#include "world/DepthOrder.h"

#include <algorithm>

namespace world {

void DepthOrder::clear() {
	items_.clear();
	hasElements_ = false;
	sortedNow_ = false;
}

void DepthOrder::addElement(int baseRow, std::uint32_t index) {
	items_.push_back(DepthItem{baseRow, DepthKind::Element, index});
	hasElements_ = true;
	sortedNow_ = false;
}

void DepthOrder::addFurniture(int baseRow, std::uint32_t index) {
	items_.push_back(DepthItem{baseRow, DepthKind::Furniture, index});
	hasElements_ = true;
	sortedNow_ = false;
}

void DepthOrder::addEntity(int row, std::uint32_t index) {
	items_.push_back(DepthItem{row, DepthKind::Entity, index});
	sortedNow_ = false;
}

bool DepthOrder::hasElements() const { return hasElements_; }

const std::vector<DepthItem>& DepthOrder::sorted() {
	// Sin elementos no hay nada que intercalar: las entidades quedan en
	// el orden en que llegaron.
	if (hasElements_ && !sortedNow_) {
		// Orden del enum: a igual fila lo fijo va antes que la entidad.
		std::stable_sort(items_.begin(), items_.end(),
				 [](const DepthItem& left, const DepthItem& right) {
					 if (left.row != right.row) {
						 return left.row < right.row;
					 }
					 return left.kind < right.kind;
				 });
		sortedNow_ = true;
	}
	return items_;
}

}
