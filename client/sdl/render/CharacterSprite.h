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
	CharacterSprite(SpriteSheet& sheet, int drawnWidth, int drawnHeight,
			int referenceCols = 0, int referenceRows = 0);

	void draw(const world::Pose& pose, const SDL2pp::Rect& logicalBox,
		  Flip flip = Flip::None) const;

private:
	SpriteSheet& sheet_;
	int drawnWidth_;
	int drawnHeight_;
	int referenceCols_;
	int referenceRows_;
};

}
