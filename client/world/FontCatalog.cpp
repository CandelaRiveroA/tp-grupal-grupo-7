#include "world/FontCatalog.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/WorldConstants.h"

namespace world {

FontCatalog::FontCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(path,
						tomlcfg::TomlDocument::Source::File);
		file = doc.getString("font.file");
		size = doc.getInt("font.size");
		faces[kDefaultFontFace] = file;
		if (doc.hasKey("font.face")) {
			for (const tomlcfg::TomlDocument& face :
			     doc.getTableArray("font.face")) {
				const std::string name = face.getString("name");
				const std::string faceFile = face.getString("file");
				if (name.empty() || faceFile.empty()) {
					throw ConfigError(ConfigErrorKind::InvalidValue,
							  "font.face");
				}
				faces[name] = faceFile;
			}
		}
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

const std::string& FontCatalog::fileFor(const std::string& face) const {
	const auto it = faces.find(face);
	if (it == faces.end()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "font.face." + face);
	}
	return it->second;
}

}  // namespace world
