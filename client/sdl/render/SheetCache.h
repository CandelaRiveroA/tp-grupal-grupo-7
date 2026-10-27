#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "sdl/render/SpriteSheet.h"

namespace sdl {

class SdlContext;

class SheetCache {
public:
	explicit SheetCache(SdlContext& context);

	SpriteSheet& sheet(const std::string& key, const std::string& image,
			   int cellWidth, int cellHeight);

private:
	SdlContext& context_;

	std::unordered_map<std::string, std::unique_ptr<SpriteSheet>>
		sheets_;
};

}
