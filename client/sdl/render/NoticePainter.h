#pragma once

#include <optional>

#include "game/NoticeQueue.h"
#include "sdl/render/SpriteSheet.h"
#include "world/NoticeLayout.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}  // namespace sdl

namespace sdl {

// Dibuja el aviso vivo (cartel 9-slice + texto). No decide cuándo hay
// aviso (game::NoticeQueue) ni dónde cae (world::NoticeLayout::boxFor):
// solo pinta.
class NoticePainter {
public:
    NoticePainter(SdlContext& context, TextRenderer& text, const world::NoticeLayout& layout);

    void draw(const std::optional<game::ActiveNotice>& notice);

private:
    SdlContext& context_;
    TextRenderer& text_;
    world::NoticeLayout layout_;
    SpriteSheet sheet_;
};

}  // namespace sdl
