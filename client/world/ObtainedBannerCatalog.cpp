#include "world/ObtainedBannerCatalog.h"

namespace world {

const SpriteVisual* ObtainedBannerCatalog::find(
    const std::string& contentId) const {
    const auto it = icons.find(contentId);
    return it == icons.end() ? nullptr : &it->second;
}

}  // namespace world
