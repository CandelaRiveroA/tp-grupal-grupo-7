#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <optional>
#include <string>

#include "world/Color.h"
#include "world/PagedModal.h"
#include "world/PixelBox.h"

namespace sdl {

class InventoryHud;
class SdlContext;
class TextRenderer;

// Dibuja la carcasa común de los modales paginados (fondo oscurecido,
// pergamino cálido, marco de madera 9-slice, flechas izquierda/derecha y
// cajas con borde). Compartido entre WorldMapPainter y CommunityPopupPainter.
class PagedModalPainter {
public:
	PagedModalPainter(SdlContext& context, InventoryHud& inventoryHud,
			  TextRenderer& text, const world::PagedModalSpec& spec);

	void drawShell(const world::PagedModalLayout& layout, int screenW,
		       int screenH);
	void drawArrows(const world::PagedModalLayout& layout);
	void fillRect(const world::PixelBox& box,
		      const world::Color& color) const;
	void drawBorderedBox(const world::PixelBox& box,
			     const world::Color& fill,
			     const world::Color& border, int borderPx) const;
	void drawTexture(SDL2pp::Texture& texture,
			 const world::PixelBox& box) const;

private:
	SDL2pp::Rect toSdlRect(const world::PixelBox& box) const;
	SDL2pp::Color toSdlColor(const world::Color& color) const;
	void drawArrow(std::optional<SDL2pp::Texture>& texture,
		       const std::string& fallbackText,
		       const world::PixelBox& box);

	SdlContext& context_;
	InventoryHud& inventoryHud_;
	TextRenderer& text_;
	world::PagedModalSpec spec_;
	std::optional<SDL2pp::Texture> leftArrowTex_;
	std::optional<SDL2pp::Texture> rightArrowTex_;
};

}  // namespace sdl
