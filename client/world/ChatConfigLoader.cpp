#include "world/ChatConfigLoader.h"

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/ChatTomlReader.h"

namespace world {

ChatLayout ChatConfigLoader::loadFromFile(
	const std::string& path) const {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		return ChatTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

}
