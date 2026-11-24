#include "sdl/render/DigitPainter.h"

#include <algorithm>
#include <cstddef>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

DigitPainter::DigitPainter(SdlContext& context,
			   const world::DigitCatalog& digits)
	: digits_(digits)
	, sheet_(context, core::assetPath(digits.sheetImage), digits.cellWidth,
		 digits.cellHeight) {}

void DigitPainter::draw(const std::vector<int>& digits,
			const std::vector<world::PixelBox>& boxes) {
	const std::size_t count = std::min(digits.size(), boxes.size());
	for (std::size_t i = 0; i < count; ++i) {
		const world::SheetCell& cell = digits_.cellFor(digits[i]);
		const world::PixelBox& box = boxes[i];
		sheet_.draw(cell.col, cell.row,
			    SDL2pp::Rect(box.x, box.y, box.width, box.height));
	}
}

}  // namespace sdl
