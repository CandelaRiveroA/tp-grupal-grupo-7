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
			const bool bold) {
	if (text.empty()) {
		return;
	}
	SDL2pp::Font& font = fontFor(size, bold);
	const SDL2pp::Color ink(static_cast<Uint8>(color.r),
				static_cast<Uint8>(color.g),
				static_cast<Uint8>(color.b),
				static_cast<Uint8>(color.a));
	SDL2pp::Surface surface = font.RenderUTF8_Blended(text, ink);
	const SDL2pp::Point measured = font.GetSizeUTF8(text);
	const int x = box.x + (box.width - measured.x) / 2;
	const int y = box.y + (box.height - measured.y) / 2;
	context_.drawSurface(surface, SDL2pp::Rect(x, y, measured.x, measured.y));
}

void TextRenderer::drawLeftWrapped(const std::string& text,
				   const world::PixelBox& box,
				   const world::Color& color, const int size,
				   const int lineGap) {
	if (text.empty() || box.width <= 0 || box.height <= 0) {
		return;
	}
	SDL2pp::Font& font = fontFor(size, false);
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

SDL2pp::Font& TextRenderer::fontFor(const int size, const bool bold) {
	const std::pair<int, bool> key{size, bold};
	const auto found = fonts_.find(key);
	if (found != fonts_.end()) {
		return fonts_.at(key);
	}
	// Una fuente abierta por (tamaño, negrita), cacheada (SDL2pp::Font no
	// es copiable: se mueve al mapa y se consulta de ahí).
	SDL2pp::Font font(core::assetPath(font_.file), size);
	if (bold) {
		font.SetStyle(TTF_STYLE_BOLD);
	}
	return fonts_.emplace(key, std::move(font)).first->second;
}

}  // namespace sdl
