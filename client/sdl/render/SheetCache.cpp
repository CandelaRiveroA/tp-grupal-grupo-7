#include "sdl/render/SheetCache.h"

#include "sdl/context/SdlContext.h"

namespace sdl {

SheetCache::SheetCache(SdlContext& context) : context_(context) {}

SpriteSheet& SheetCache::sheet(const std::string& key,
			       const std::string& image, int cellWidth,
			       int cellHeight) {
	const auto found = sheets_.find(key);
	if (found != sheets_.end()) {
		return *found->second;
	}
	auto sheet = std::make_unique<SpriteSheet>(context_, image,
						   cellWidth, cellHeight);
	SpriteSheet& reference = *sheet;
	sheets_.emplace(key, std::move(sheet));
	return reference;
}

}
