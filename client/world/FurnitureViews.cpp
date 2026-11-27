#include "world/FurnitureViews.h"

#include <algorithm>

namespace world {

FurnitureViews::FurnitureViews(const ContentCatalog& content)
	: content_(content)
	, rows_{
		  // Si el sprite `side` de la hoja mira al otro lado, se
		  // intercambia `mirrored` entre Right y Left (solo datos).
		  {FurnitureFacing::Front, Row{"", false, false}},
		  {FurnitureFacing::Right, Row{"_side", false, true}},
		  {FurnitureFacing::Back, Row{"_back", false, false}},
		  {FurnitureFacing::Left, Row{"_side", true, true}},
	  }
	, cycle_{FurnitureFacing::Front, FurnitureFacing::Right,
		 FurnitureFacing::Back, FurnitureFacing::Left} {}

bool FurnitureViews::has(const std::string& contentId,
			 const FurnitureFacing facing) const {
	return content_.find(contentId + rows_.at(facing).suffix) != nullptr;
}

FurnitureFacing FurnitureViews::next(const std::string& contentId,
				     const FurnitureFacing current) const {
	const auto at = std::find(cycle_.begin(), cycle_.end(), current);
	const std::size_t start =
		at == cycle_.end() ? 0 : static_cast<std::size_t>(at - cycle_.begin());
	for (std::size_t step = 1; step <= cycle_.size(); ++step) {
		const FurnitureFacing candidate = cycle_[(start + step) % cycle_.size()];
		if (has(contentId, candidate)) {
			return candidate;
		}
	}
	return current;
}

FurnitureView FurnitureViews::viewOf(const std::string& contentId,
				     const FurnitureFacing facing) const {
	if (!has(contentId, facing)) {
		return FurnitureView{content_.find(contentId), false};
	}
	const Row& row = rows_.at(facing);
	return FurnitureView{content_.find(contentId + row.suffix), row.mirrored};
}

GridSpan FurnitureViews::footprintOf(const GridSpan& span,
				     const FurnitureFacing facing) const {
	return rows_.at(facing).swapsFootprint ? GridSpan{span.rows, span.columns}
						: span;
}

}
