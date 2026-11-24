#include "sdl/render/TextRenderer.h"

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

TextRenderer::TextRenderer(SdlContext& context, const world::FontCatalog& font)
	: context_(context), font_(font) {}

void TextRenderer::draw(const std::string& text, const world::PixelBox& box,
			const world::Color& color) {
	draw(text, box, color, font_.size);
}

void TextRenderer::draw(const std::string& text, const world::PixelBox& box,
			const world::Color& color, const int size) {
	draw(text, box, color, size, false);
}

void TextRenderer::draw(const std::string& text, const world::PixelBox& box,
			const world::Color& color, const int size,
			const bool bold, const std::string& face) {
	const int x = box.x + (box.width - textWidth(text, size, bold, face)) / 2;
	drawLeft(text, world::PixelBox{x, box.y, box.width, box.height}, color,
		 size, bold, face);
}

void TextRenderer::drawLeft(const std::string& text,
			    const world::PixelBox& box,
			    const world::Color& color, const int size,
			    const bool bold, const std::string& face) {
	if (text.empty()) {
		return;
	}
	SDL2pp::Font& font = fontFor(face, size, bold);
	const SDL2pp::Color ink(static_cast<Uint8>(color.r),
				static_cast<Uint8>(color.g),
				static_cast<Uint8>(color.b),
				static_cast<Uint8>(color.a));
	SDL2pp::Surface surface = font.RenderUTF8_Blended(text, ink);
	const SDL2pp::Point measured = font.GetSizeUTF8(text);
	const int y = box.y + (box.height - measured.y) / 2;
	context_.drawSurface(surface, SDL2pp::Rect(box.x, y, measured.x, measured.y));
}

int TextRenderer::textWidth(const std::string& text, const int size,
			    const bool bold, const std::string& face) {
	return fontFor(face, size, bold).GetSizeUTF8(text).x;
}

SDL2pp::Texture TextRenderer::renderWrapped(const std::string& text,
					    const int size,
					    const world::Color& color,
					    const int maxWidth,
					    const std::string& face) {
	const SDL2pp::Color ink(static_cast<Uint8>(color.r),
				static_cast<Uint8>(color.g),
				static_cast<Uint8>(color.b),
				static_cast<Uint8>(color.a));
	SDL_Surface* raw = TTF_RenderUTF8_Blended_Wrapped(
		fontFor(face, size, false).Get(), text.c_str(), ink, maxWidth);
	// Un texto sin glifos da superficie nula: un espacio la mantiene
	// renderizable (la caja de tipeo vacía necesita su alto).
	if (raw == nullptr) {
		raw = TTF_RenderUTF8_Blended_Wrapped(fontFor(face, size, false).Get(), " ",
						     ink, maxWidth);
	}
	if (raw == nullptr) {
		throw SDL2pp::Exception("TTF_RenderUTF8_Blended_Wrapped");
	}
	SDL2pp::Surface surface(raw);
	return context_.createTexture(surface);
}

void TextRenderer::drawLeftWrapped(const std::string& text,
				   const world::PixelBox& box,
				   const world::Color& color, const int size,
				   const int lineGap, const std::string& face) {
	if (text.empty() || box.width <= 0 || box.height <= 0) {
		return;
	}
	SDL2pp::Font& font = fontFor(face, size, false);
	const SDL2pp::Color ink(static_cast<Uint8>(color.r),
				static_cast<Uint8>(color.g),
				static_cast<Uint8>(color.b),
				static_cast<Uint8>(color.a));
	const int lineHeight = font.GetHeight();
	const int step = lineHeight + lineGap;
	int y = box.y;

	std::size_t start = 0;
	while (start <= text.size()) {
		const std::size_t end = text.find('\n', start);
		const std::string paragraph =
			text.substr(start, end == std::string::npos
						   ? std::string::npos
						   : end - start);
		std::size_t indent = 0;
		while (indent < paragraph.size() && paragraph[indent] == ' ') {
			++indent;
		}
		std::string line = paragraph.substr(0, indent);
		std::size_t pos = indent;
		while (pos < paragraph.size()) {
			while (pos < paragraph.size() && paragraph[pos] == ' ') {
				++pos;
			}
			if (pos >= paragraph.size()) {
				break;
			}
			std::size_t wordEnd = pos;
			while (wordEnd < paragraph.size() &&
			       paragraph[wordEnd] != ' ') {
				++wordEnd;
			}
			const std::string word =
				paragraph.substr(pos, wordEnd - pos);
			const std::string candidate =
				line.find_first_not_of(' ') == std::string::npos
					? line + word
					: line + " " + word;
			if (line.find_first_not_of(' ') == std::string::npos ||
			    font.GetSizeUTF8(candidate).x <= box.width) {
				line = candidate;
			} else {
				drawLine(line, box.x, y, font, ink);
				y += step;
				line = word;
			}
			pos = wordEnd;
		}
		if (!line.empty()) {
			if (y + lineHeight > box.y + box.height) {
				return;
			}
			drawLine(line, box.x, y, font, ink);
		}
		y += step;
		if (end == std::string::npos ||
		    y + lineHeight > box.y + box.height) {
			return;
		}
		start = end + 1;
	}
}

void TextRenderer::drawLine(const std::string& line, const int x,
			    const int y, SDL2pp::Font& font,
			    const SDL2pp::Color& ink) {
	if (line.empty()) {
		return;
	}
	SDL2pp::Surface surface = font.RenderUTF8_Blended(line, ink);
	const SDL2pp::Point measured = font.GetSizeUTF8(line);
	context_.drawSurface(surface,
			     SDL2pp::Rect(x, y, measured.x, measured.y));
}

SDL2pp::Font& TextRenderer::fontFor(const std::string& face, const int size,
				    const bool bold) {
	const std::tuple<std::string, int, bool> key{face, size, bold};
	const auto found = fonts_.find(key);
	if (found != fonts_.end()) {
		return fonts_.at(key);
	}
	// Una fuente abierta por (cara, tamaño, negrita), cacheada
	// (SDL2pp::Font no es copiable: se mueve al mapa y se consulta de ahí).
	SDL2pp::Font font(core::assetPath(font_.fileFor(face)), size);
	if (bold) {
		font.SetStyle(TTF_STYLE_BOLD);
	}
	return fonts_.emplace(key, std::move(font)).first->second;
}

}  // namespace sdl
