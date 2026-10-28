#include "world/ObtainedBannerCatalog.h"

namespace world {

bool ObtainedBannerCatalog::hasIcon(ResourceKind kind) const {
    return icons.find(kind) != icons.end();
}

const BannerIcon& ObtainedBannerCatalog::iconFor(ResourceKind kind) const { return icons.at(kind); }

}  // namespace world
