#include "SpriteSheet.h"

#include "sdl/context/SdlContext.h"

namespace sdl {

SpriteSheet::SpriteSheet(SdlContext& context, const std::string& path,
			 int frameWidth, int frameHeight)
	: context_(context)
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
