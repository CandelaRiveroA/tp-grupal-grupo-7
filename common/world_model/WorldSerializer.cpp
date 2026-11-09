#include "world_model/WorldSerializer.h"

#include "toml/TomlDocument.h"
#include "world_model/FormatError.h"
#include "world_model/WorldTomlReader.h"

namespace worldmodel {

namespace {

World readDocument(const std::string& content,
		   tomlcfg::TomlDocument::Source source) {
	try {
		const tomlcfg::TomlDocument doc(content, source);
		return WorldTomlReader(doc).read();
	} catch (const tomlcfg::TomlError& error) {
		// Los FormatError semánticos se propagan tal cual.
		throw FormatError(error);
	}
}

}

World WorldSerializer::loadFromFile(const std::string& path) const {
	return readDocument(path, tomlcfg::TomlDocument::Source::File);
}

World WorldSerializer::loadFromString(const std::string& toml) const {
	return readDocument(toml, tomlcfg::TomlDocument::Source::Text);
}

}
