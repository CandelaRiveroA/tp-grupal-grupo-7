#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SpriteSheet.h"
#include "world/ChatLayout.h"
#include "world/ChatWrap.h"
#include "world/PixelBox.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Dibuja el chatbox abierto (historial + tipeo) o su preview atenuado.
class ChatPainter {
public:
	ChatPainter(SdlContext& context, TextRenderer& text,
		    const world::ChatLayout& layout);

	// cursorOn lo decide el tick de la sesión (el parpadeo).
	void drawOpen(const std::string& inputText,
		      const std::vector<game::ViewChatLine>& log, bool cursorOn);

	void drawPreview(const std::vector<game::ViewChatLine>& log);

private:
	// Un solo camino de dibujo: textAlpha es del TEXTO (la caja ya viene
	// semitransparente) e inputLines vacío = sin caja de tipeo (preview).
	void drawLines(const std::vector<std::string>& inputLines,
		       const std::vector<std::string>& historyLines, bool cursorOn,
		       std::uint8_t textAlpha);

	void drawBox(const world::PixelBox& box);
	void drawLine(const world::PixelBox& panel, int row,
		      const std::string& text, std::uint8_t textAlpha);

	// Firma del historial: el nombre del personaje o, si no llegó, el id.
	static std::string withSender(const game::ViewChatLine& line);

	// Margen del texto desde el borde de la caja (incluye el marco).
	int textInset() const;

	int contentWidth() const;

	world::TextMeasure measure() const;

	std::vector<std::vector<std::string>> wrapMessages(
		const std::vector<game::ViewChatLine>& log) const;

	SdlContext& context_;
	TextRenderer& text_;
	world::ChatLayout layout_;
	SpriteSheet sheet_;
};

}