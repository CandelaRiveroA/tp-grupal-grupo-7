#pragma once

#include <SDL2pp/SDL2pp.hh>

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

	void drawBlock(int col, int row, int cols, int rows,
		       const SDL2pp::Rect& dest, Flip flip = Flip::None,
		       double fracW = 1.0, double fracH = 1.0);

private:
	SdlContext& context_;
	SDL2pp::Texture texture_;
	int frameWidth_;
	int frameHeight_;
};

}
