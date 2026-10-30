#pragma once

#include <string>

#include "world/ObtainedBannerCatalog.h"

namespace world {

// Carga obtained_banner.toml hacia un ObtainedBannerCatalog. Misma forma
// que ResourceConfigLoader: desde archivo o desde string, traduciendo
// TomlError a ConfigError.
class ObtainedBannerConfigLoader {
public:
    ObtainedBannerCatalog loadFromFile(const std::string& path) const;

    ObtainedBannerCatalog loadFromString(const std::string& toml) const;
};

}  // namespace world
