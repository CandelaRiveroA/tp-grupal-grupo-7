#include "SpriteSheet.h"

#include <algorithm>

#include "sdl/context/SdlContext.h"

namespace sdl {

SpriteSheet::SpriteSheet(SdlContext& context, const std::string& path,
			 int frameWidth, int frameHeight)
	: context_(context)
	, path_(path)
	, texture_(context.createTexture(path))
	, frameWidth_(frameWidth)
	, frameHeight_(frameHeight) {}

int SpriteSheet::frameWidth() const { return frameWidth_; }

int SpriteSheet::frameHeight() const { return frameHeight_; }

SDL2pp::Rect SpriteSheet::frameRect(int col, int row) const {
	return SDL2pp::Rect(col * frameWidth_, row * frameHeight_,
			    frameWidth_, frameHeight_);
}

void SpriteSheet::draw(int col, int row, const SDL2pp::Rect& dest,
		       Flip flip) {
	context_.drawTexture(texture_, frameRect(col, row), dest, flip);
}

void SpriteSheet::drawContentCentered(const int col, const int row,
				      const SDL2pp::Rect& dest,
				      const double scale) {
	if (dest.w <= 0 || dest.h <= 0 || scale <= 0.0) {
		return;
	}
	// El contenido va a la MISMA escala que tendría la celda entera
	// (dest / celda), con la escala pedida encima, y centrado en dest.
	const SDL2pp::Rect content = contentRect(col, row);
	const int width = static_cast<int>(content.w * dest.w * scale /
					   frameWidth_);
	const int height = static_cast<int>(content.h * dest.h * scale /
					    frameHeight_);
	if (width <= 0 || height <= 0) {
		return;
	}
	context_.drawTexture(
		texture_, content,
		SDL2pp::Rect(dest.x + (dest.w - width) / 2,
			     dest.y + (dest.h - height) / 2, width, height),
		Flip::None);
}

void SpriteSheet::drawContentStretched(const int col, const int row,
				       const SDL2pp::Rect& dest) {
	if (dest.w <= 0 || dest.h <= 0) {
		return;
	}
	context_.drawTexture(texture_, contentRect(col, row), dest, Flip::None);
}

SDL2pp::Rect SpriteSheet::contentRect(const int col, const int row) {
	const auto key = std::make_pair(col, row);
	const auto cached = contentRects_.find(key);
	if (cached != contentRects_.end()) {
		return cached->second;
	}
	const SDL2pp::Rect rect = scanContentRect(col, row);
	contentRects_.emplace(key, rect);
	return rect;
}

SDL2pp::Rect SpriteSheet::scanContentRect(const int col, const int row) {
	if (!surface_.has_value()) {
		surface_.emplace(path_);
	}
	SDL2pp::Surface::LockHandle lock = surface_->Lock();
	const SDL_PixelFormat& format = lock.GetFormat();
	if (format.BytesPerPixel != 4) {
		return frameRect(col, row);  // sin alfa consultable
	}
	const int baseX = col * frameWidth_;
	const int baseY = row * frameHeight_;
	const auto* pixels =
		static_cast<const std::uint32_t*>(lock.GetPixels());
	const int pitch = lock.GetPitch() / static_cast<int>(sizeof(std::uint32_t));
	int minX = frameWidth_;
	int minY = frameHeight_;
	int maxX = -1;
	int maxY = -1;
	for (int y = 0; y < frameHeight_; ++y) {
		for (int x = 0; x < frameWidth_; ++x) {
			const std::uint32_t pixel =
				pixels[(baseY + y) * pitch + baseX + x];
			std::uint8_t r = 0;
			std::uint8_t g = 0;
			std::uint8_t b = 0;
			std::uint8_t a = 0;
			SDL_GetRGBA(pixel, &format, &r, &g, &b, &a);
			if (a == 0) {
				continue;
			}
			minX = std::min(minX, x);
			minY = std::min(minY, y);
			maxX = std::max(maxX, x);
			maxY = std::max(maxY, y);
		}
	}
	if (maxX < 0) {
		return frameRect(col, row);  // celda vacía
	}
	return SDL2pp::Rect(baseX + minX, baseY + minY, maxX - minX + 1,
			    maxY - minY + 1);
}

void SpriteSheet::setColorMod(std::uint8_t r, std::uint8_t g,
			      std::uint8_t b) {
	texture_.SetColorMod(r, g, b);
}

void SpriteSheet::clearColorMod() { texture_.SetColorMod(255, 255, 255); }

void SpriteSheet::drawBlock(int col, int row, int cols, int rows,
			    const SDL2pp::Rect& dest, Flip flip,
			    double fracW, double fracH, double angle,
			    SDL2pp::Optional<SDL2pp::Point> center) {
	if (cols <= 0 || rows <= 0 || dest.w <= 0 || dest.h <= 0) {
		return;
	}
	const SDL2pp::Rect base = frameRect(col, row);
	const int fullW = cols * frameWidth_;
	const int fullH = rows * frameHeight_;
	const int srcW = static_cast<int>(fullW * fracW);
	const int srcH = static_cast<int>(fullH * fracH);
	if (srcW <= 0 || srcH <= 0) {
		return;
	}
	const SDL2pp::Rect src(base.x, base.y, srcW, srcH);
	context_.drawTexture(texture_, src, dest, flip, angle, center);
}

}
