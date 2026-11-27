#include "sdl/render/PixelComposer.h"

#include <cstddef>

#include "sdl/context/SdlContext.h"
#include "sdl/render/SpriteSheet.h"
#include "world/SpriteVisual.h"

namespace sdl {

void PixelComposer::PixelRules::merge(const PixelRules& other) {
	recolors.insert(recolors.end(), other.recolors.begin(),
			other.recolors.end());
}

PixelComposer::PixelComposer(SdlContext& context) : context_(context) {}

void PixelComposer::reset(const int width, const int height) {
	width_ = width;
	height_ = height;
	canvas_.assign(static_cast<std::size_t>(width) *
			       static_cast<std::size_t>(height),
		       0);
	if (!texture_.has_value() || textureWidth_ != width ||
	    textureHeight_ != height) {
		texture_.emplace(context_.createStreamingTexture(
			width, height, SDL_PIXELFORMAT_ARGB8888));
		texture_->SetBlendMode(SDL_BLENDMODE_BLEND);
		textureWidth_ = width;
		textureHeight_ = height;
	}
}

void PixelComposer::drawRegion(SpriteSheet& sheet,
			       const world::SheetRegion& region,
			       const int offsetX, const int offsetY,
			       const PixelRules& rules) {
	if (region.cols <= 0 || region.rows <= 0) {
		return;
	}
	SDL2pp::Surface& surface = sheet.surface();
	SDL2pp::Surface::LockHandle lock = surface.Lock();
	const SDL_PixelFormat& format = lock.GetFormat();
	if (format.BytesPerPixel != 4) {
		return;
	}

	const int frameWidth = sheet.frameWidth();
	const int frameHeight = sheet.frameHeight();
	const int srcX = region.at.col * frameWidth;
	const int srcY = region.at.row * frameHeight;
	const int width = region.cols * frameWidth;
	const int height = region.rows * frameHeight;
	const int surfaceWidth = surface.GetWidth();
	const int surfaceHeight = surface.GetHeight();
	const auto* pixels =
		static_cast<const std::uint32_t*>(lock.GetPixels());
	const int pitch =
		lock.GetPitch() / static_cast<int>(sizeof(std::uint32_t));

	for (int y = 0; y < height; ++y) {
		const int sourceY = srcY + y;
		if (sourceY < 0 || sourceY >= surfaceHeight) {
			continue;
		}
		for (int x = 0; x < width; ++x) {
			const int sourceX = srcX + x;
			if (sourceX < 0 || sourceX >= surfaceWidth) {
				continue;
			}
			Rgba pixel;
			SDL_GetRGBA(pixels[sourceY * pitch + sourceX], &format,
				    &pixel.r, &pixel.g, &pixel.b, &pixel.a);
			blendPixel(offsetX + x, offsetY + y,
				   applyRules(pixel, rules));
		}
	}
}

void PixelComposer::recolorCanvas(const PixelRules& rules) {
	for (std::uint32_t& pixel : canvas_) {
		Rgba color = unpack(pixel);
		if (color.a != 255) {
			continue;
		}
		pixel = pack(applyRules(color, rules));
	}
}

PixelComposer::Rgba PixelComposer::sample(SpriteSheet& sheet, const int x,
					  const int y) const {
	SDL2pp::Surface& surface = sheet.surface();
	if (x < 0 || y < 0 || x >= surface.GetWidth() ||
	    y >= surface.GetHeight()) {
		return Rgba{};
	}
	SDL2pp::Surface::LockHandle lock = surface.Lock();
	const SDL_PixelFormat& format = lock.GetFormat();
	if (format.BytesPerPixel != 4) {
		return Rgba{};
	}
	const auto* pixels =
		static_cast<const std::uint32_t*>(lock.GetPixels());
	const int pitch =
		lock.GetPitch() / static_cast<int>(sizeof(std::uint32_t));
	Rgba color;
	SDL_GetRGBA(pixels[y * pitch + x], &format, &color.r, &color.g,
		    &color.b, &color.a);
	return color;
}

void PixelComposer::present(const SDL2pp::Rect& dest, const Flip flip) {
	presentRegion(SDL2pp::Rect(0, 0, width_, height_), dest, flip);
}

void PixelComposer::presentRegion(const SDL2pp::Rect& src,
				  const SDL2pp::Rect& dest, const Flip flip) {
	if (!texture_.has_value() || width_ <= 0 || height_ <= 0) {
		return;
	}
	texture_->Update(SDL2pp::NullOpt, canvas_.data(),
			 width_ * static_cast<int>(sizeof(std::uint32_t)));
	context_.drawTexture(*texture_, src, dest, flip);
}

PixelComposer::Rgba PixelComposer::applyRules(Rgba pixel,
					      const PixelRules& rules) const {
	if (pixel.a == 255) {
		for (const Recolor& rule : rules.recolors) {
			if (pixel.r == rule.from.r && pixel.g == rule.from.g &&
			    pixel.b == rule.from.b) {
				if (rule.to.a == 255) {
					pixel.r = rule.to.r;
					pixel.g = rule.to.g;
					pixel.b = rule.to.b;
				}
				break;
			}
		}
	}
	if (rules.tint) {
		pixel.r = static_cast<std::uint8_t>(
			static_cast<int>(pixel.r) * rules.tintColor.r / 255);
		pixel.g = static_cast<std::uint8_t>(
			static_cast<int>(pixel.g) * rules.tintColor.g / 255);
		pixel.b = static_cast<std::uint8_t>(
			static_cast<int>(pixel.b) * rules.tintColor.b / 255);
	}
	return pixel;
}

void PixelComposer::blendPixel(const int x, const int y,
			       const Rgba& color) {
	if (color.a == 0 || x < 0 || y < 0 || x >= width_ || y >= height_) {
		return;
	}
	std::uint32_t& destination =
		canvas_[static_cast<std::size_t>(y) *
				static_cast<std::size_t>(width_) +
			static_cast<std::size_t>(x)];
	if (color.a == 255) {
		destination = pack(color);
		return;
	}

	const Rgba under = unpack(destination);
	const int sourceAlpha = color.a;
	const int underAlpha = under.a;
	const int inverse = 255 - sourceAlpha;
	const int outAlpha = sourceAlpha + underAlpha * inverse / 255;
	if (outAlpha == 0) {
		destination = 0;
		return;
	}
	const auto blend = [&](const int source, const int base) {
		return (source * sourceAlpha +
			base * underAlpha * inverse / 255) /
		       outAlpha;
	};
	const Rgba out{
		static_cast<std::uint8_t>(blend(color.r, under.r)),
		static_cast<std::uint8_t>(blend(color.g, under.g)),
		static_cast<std::uint8_t>(blend(color.b, under.b)),
		static_cast<std::uint8_t>(outAlpha),
	};
	destination = pack(out);
}

std::uint32_t PixelComposer::pack(const Rgba& color) const {
	return (static_cast<std::uint32_t>(color.a) << 24) |
	       (static_cast<std::uint32_t>(color.r) << 16) |
	       (static_cast<std::uint32_t>(color.g) << 8) |
	       static_cast<std::uint32_t>(color.b);
}

PixelComposer::Rgba PixelComposer::unpack(const std::uint32_t pixel) const {
	return Rgba{
		static_cast<std::uint8_t>((pixel >> 16) & 0xFF),
		static_cast<std::uint8_t>((pixel >> 8) & 0xFF),
		static_cast<std::uint8_t>(pixel & 0xFF),
		static_cast<std::uint8_t>((pixel >> 24) & 0xFF),
	};
}

}  // namespace sdl
