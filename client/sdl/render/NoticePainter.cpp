#include "sdl/render/NoticePainter.h"

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

NoticePainter::NoticePainter(SdlContext& context, TextRenderer& text,
                             const world::NoticeLayout& layout):
        context_(context),
        text_(text),
        layout_(layout),
        sheet_(context, core::assetPath(layout.sheetImage), layout.cellWidth,
               layout.cellHeight) {}

void NoticePainter::draw(const std::optional<game::ActiveNotice>& notice) {
    if (!notice.has_value()) {
        return;
    }
    const std::string* message = layout_.textFor(notice->id);
    if (message == nullptr) {
        return;
    }
    const int textWidth = text_.textWidth(*message, layout_.fontSize, false, layout_.fontFace);
    const world::PixelBox box =
            layout_.boxFor(textWidth, context_.width(), context_.height());
    sheet_.drawNineSlice(0, 0, layout_.border,
                         SDL2pp::Rect(box.x, box.y, box.width, box.height));
    text_.draw(*message, box, layout_.textColor, layout_.fontSize, false, layout_.fontFace);
}

}  // namespace sdl
