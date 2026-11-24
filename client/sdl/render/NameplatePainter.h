#pragma once

#include <string>

#include "world/NameplateLayout.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class TextRenderer;
class Viewport;
}  // namespace sdl

namespace sdl {

// Dibuja el nombre bajo los pies de una entidad. No decide QUÉ nombre
// (game::ViewEntity::displayName, de la fuente de estado), ni dónde cae
// ni de qué color (world::NameplateLayout): mide, traduce por el
// viewport (§11) y pinta con TextRenderer.
class NameplatePainter {
public:
    NameplatePainter(TextRenderer& text, const world::NameplateLayout& layout);

    // drawn: rectángulo dibujado de la entidad (px de mundo, el de
    // ViewMotion::drawBoxFor). hour: hora cruda del reloj del juego.
    void draw(const std::string& name, const world::PixelRect& drawn,
              const Viewport& viewport, int hour);

private:
    TextRenderer& text_;
    world::NameplateLayout layout_;
};

}  // namespace sdl
