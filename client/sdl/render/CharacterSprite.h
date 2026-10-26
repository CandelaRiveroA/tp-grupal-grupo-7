#pragma once

#include <SDL2pp/SDL2pp.hh>

#include "sdl/context/SdlConstants.h"
#include "world/CharacterAppearance.h"

namespace sdl {
class SpriteSheet;
}

namespace sdl {

class CharacterSprite {
public:
	CharacterSprite(SpriteSheet& sheet, int drawnWidth, int drawnHeight);

	void draw(const world::Pose& pose, const SDL2pp::Rect& logicalBox,
		  Flip flip = Flip::None);

private:
	SpriteSheet& sheet_;
	int drawnWidth_;
	int drawnHeight_;
};

}
