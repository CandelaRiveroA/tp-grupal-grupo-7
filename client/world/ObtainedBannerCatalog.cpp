#include "world/ObtainedBannerCatalog.h"

namespace world {

bool ObtainedBannerCatalog::hasIcon(ResourceKind kind) const {
    return icons.find(kind) != icons.end();
}

const SpriteVisual& ObtainedBannerCatalog::iconFor(ResourceKind kind) const { return icons.at(kind); }

}  // namespace world
