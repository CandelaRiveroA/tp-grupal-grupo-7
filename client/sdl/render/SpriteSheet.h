#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <cstdint>
#include <string>

#include "sdl/context/SdlConstants.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class SpriteSheet {
public:
	SpriteSheet(SdlContext& context, const std::string& path,
		    int frameWidth, int frameHeight);

	SpriteSheet(const SpriteSheet&) = delete;
	SpriteSheet& operator=(const SpriteSheet&) = delete;

	int frameWidth() const;
	int frameHeight() const;

	SDL2pp::Rect frameRect(int col, int row) const;

	void draw(int col, int row, const SDL2pp::Rect& dest,
		  Flip flip = Flip::None);

	// Tiñe la hoja al dibujar (color modulation): cada píxel se multiplica
	// por (r,g,b). Útil para recolorear sprites claros (ej. dígitos
	// blancos -> negro con (0,0,0)). clearColorMod() vuelve a (255,255,255).
	void setColorMod(std::uint8_t r, std::uint8_t g, std::uint8_t b);
	void clearColorMod();

	void drawBlock(int col, int row, int cols, int rows,
		       const SDL2pp::Rect& dest, Flip flip = Flip::None,
		       double fracW = 1.0, double fracH = 1.0,
		       double angle = 0.0,
		       SDL2pp::Optional<SDL2pp::Point> center =
			       SDL2pp::NullOpt);

private:
	SdlContext& context_;
	SDL2pp::Texture texture_;
	int frameWidth_;
	int frameHeight_;
};

}
