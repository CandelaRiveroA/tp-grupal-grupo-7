#pragma once

#include <memory>
#include <string>
#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SpriteSheet.h"
#include "world/ChatLayout.h"
#include "world/PixelBox.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Dibuja el chatbox abierto (historial + tipeo) o su preview atenuado.
// Cada mensaje es una textura envuelta por SDL_ttf
class ChatPainter {
public:
	ChatPainter(SdlContext& context, TextRenderer& text,
		    const world::ChatLayout& layout);

	// cursorOn lo decide el tick de la sesión (el parpadeo).
	void drawOpen(const std::string& inputText,
		      const std::vector<game::ViewChatLine>& log, bool cursorOn);

	void drawPreview(const std::vector<game::ViewChatLine>& log);

private:
	// Texturas del historial (de la vieja a la nueva), reconstruidas
	// si el log cambió.
	const std::vector<std::unique_ptr<SDL2pp::Texture>>& historyTextures(
		const std::vector<game::ViewChatLine>& log);

	// Textura del tipeo, cacheada por texto (con o sin cursor).
	SDL2pp::Texture& inputTexture(const std::string& inputText,
				      bool cursorOn);

	// Apilado de abajo hacia arriba
	void drawHistory(const world::PixelBox& box, std::uint8_t textAlpha);

	void drawBox(const world::PixelBox& box);

	// Firma del historial: el nombre del personaje o, si no llegó, el id.
	static std::string withSender(const game::ViewChatLine& line);

	// Margen del texto desde el borde de la caja (incluye el marco).
	int textInset() const;

	int contentWidth() const;

	SdlContext& context_;
	TextRenderer& text_;
	world::ChatLayout layout_;
	SpriteSheet sheet_;

	std::vector<std::unique_ptr<SDL2pp::Texture>> history_;
	std::vector<std::string> historyKey_;
	std::unique_ptr<SDL2pp::Texture> input_;
	std::string inputKey_;
};

}
