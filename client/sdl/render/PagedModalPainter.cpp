#include "sdl/render/PagedModalPainter.h"

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

PagedModalPainter::PagedModalPainter(SdlContext& context,
				     InventoryHud& inventoryHud,
				     TextRenderer& text,
				     const world::PagedModalSpec& spec)
	: context_(context)
	, inventoryHud_(inventoryHud)
	, text_(text)
	, spec_(spec)
	, leftArrowTex_(
		  context.tryLoadTexture(core::assetPath(spec_.arrowLeftSheet)))
	, rightArrowTex_(context.tryLoadTexture(
		  core::assetPath(spec_.arrowRightSheet))) {}

void PagedModalPainter::drawShell(const world::PagedModalLayout& layout,
				  const int screenW, const int screenH) {
	context_.fillRect(SDL2pp::Rect(0, 0, screenW, screenH),
			  toSdlColor(spec_.backdropColor));
	fillRect(layout.backgroundBox(), spec_.parchmentColor);
	// drawPanel(0, 0) dibuja el marco de madera sin grilla de slots.
	inventoryHud_.drawPanel(layout.panel(), 0, 0);
}

void PagedModalPainter::drawArrows(const world::PagedModalLayout& layout) {
	drawArrow(leftArrowTex_, spec_.arrowLeftFallback,
		  layout.leftArrowRect());
	drawArrow(rightArrowTex_, spec_.arrowRightFallback,
		  layout.rightArrowRect());
}

void PagedModalPainter::fillRect(const world::PixelBox& box,
				 const world::Color& color) const {
	context_.fillRect(toSdlRect(box), toSdlColor(color));
}

void PagedModalPainter::drawBorderedBox(const world::PixelBox& box,
					const world::Color& fill,
					const world::Color& border,
					const int borderPx) const {
	if (borderPx > 0) {
		fillRect(box, border);
	}
	const world::PixelBox inner = box.inset(borderPx);
	if (inner.width > 0 && inner.height > 0) {
		fillRect(inner, fill);
	}
}

void PagedModalPainter::drawTexture(SDL2pp::Texture& texture,
				    const world::PixelBox& box) const {
	context_.drawTexture(
		texture,
		SDL2pp::Rect(0, 0, texture.GetWidth(), texture.GetHeight()),
		toSdlRect(box));
}

SDL2pp::Rect PagedModalPainter::toSdlRect(const world::PixelBox& box) const {
	return SDL2pp::Rect(box.x, box.y, box.width, box.height);
}

SDL2pp::Color PagedModalPainter::toSdlColor(const world::Color& color) const {
	return SDL2pp::Color(static_cast<Uint8>(color.r),
			     static_cast<Uint8>(color.g),
			     static_cast<Uint8>(color.b),
			     static_cast<Uint8>(color.a));
}

void PagedModalPainter::drawArrow(std::optional<SDL2pp::Texture>& texture,
				  const std::string& fallbackText,
				  const world::PixelBox& box) {
	if (texture.has_value()) {
		drawTexture(*texture, box);
		return;
	}
	text_.draw(fallbackText, box, spec_.inkColor, spec_.arrowFontSize,
		   true);
}

}  // namespace sdl
