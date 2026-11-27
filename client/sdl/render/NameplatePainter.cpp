#include "sdl/render/NameplatePainter.h"

#include "sdl/render/TextRenderer.h"
#include "sdl/render/Viewport.h"

namespace sdl {

NameplatePainter::NameplatePainter(TextRenderer& text, const world::NameplateLayout& layout):
        text_(text), layout_(layout) {}

void NameplatePainter::draw(const std::string& name, const world::PixelRect& drawn,
                            const Viewport& viewport, const int hour) {
    if (name.empty()) {
        return;
    }
    const int width =
            text_.textWidth(name, layout_.fontSize(), layout_.bold(), layout_.fontFace());
    const world::PixelRect screen = viewport.toScreen(layout_.boxUnder(drawn, width));
    // Culling por contrato (§11): fuera del viewport no se emite el draw.
    if (!viewport.intersects(screen)) {
        return;
    }
    text_.draw(name, world::PixelBox{screen.x, screen.y, screen.width, screen.height},
               layout_.colorAt(hour), layout_.fontSize(), layout_.bold(), layout_.fontFace());
}

}  // namespace sdl
