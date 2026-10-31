#pragma once

#include <array>
#include <string>

#include "toml/TomlDocument.h"
#include "world/ObtainedBannerCatalog.h"

namespace world {

// Traduce obtained_banner.toml a un ObtainedBannerCatalog (datos puros).
// Mismo patrón que ResourceTomlReader/InventoryTomlReader: clase propia,
// métodos de instancia, claves como constantes (en el .cpp), validación
// con ConfigError, sin funciones libres.
class ObtainedBannerTomlReader {
public:
    explicit ObtainedBannerTomlReader(const tomlcfg::TomlDocument& doc);

    ObtainedBannerCatalog read() const;

private:
    std::string join(const std::string& base, const std::string& key) const;

    BannerCell readCell(const std::string& base, const std::string& key) const;

    BannerFrame readFrame() const;

    SpriteVisual readIcon(const std::string& base) const;

    const tomlcfg::TomlDocument& doc_;
};

}  // namespace world
