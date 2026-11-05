#include "world/FontCatalog.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

FontCatalog::FontCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(path,
						tomlcfg::TomlDocument::Source::File);
		file = doc.getString("font.file");
		size = doc.getInt("font.size");
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
	if (file.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "font.file");
	}
	if (size <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "font.size");
	}
}

}  // namespace world
