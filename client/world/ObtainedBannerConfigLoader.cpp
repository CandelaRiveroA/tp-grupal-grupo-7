#include "world/ObtainedBannerConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/ObtainedBannerTomlReader.h"

namespace world {

ObtainedBannerCatalog ObtainedBannerConfigLoader::loadFromFile(const std::string& path) const {
    try {
        const tomlcfg::TomlDocument doc(path, tomlcfg::TomlDocument::Source::File);
        return ObtainedBannerTomlReader(doc).read();
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
}

ObtainedBannerCatalog ObtainedBannerConfigLoader::loadFromString(const std::string& toml) const {
    try {
        const tomlcfg::TomlDocument doc(toml, tomlcfg::TomlDocument::Source::Text);
        return ObtainedBannerTomlReader(doc).read();
    } catch (const tomlcfg::TomlError& error) {
        throw ConfigError(error);
    }
}

}  // namespace world
