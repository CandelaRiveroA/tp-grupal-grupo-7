#include "world_model/WorldSerializer.h"

#include <fstream>

#include "toml/TomlDocument.h"
#include "world_model/FormatError.h"
#include "world_model/WorldTomlReader.h"
#include "world_model/WorldTomlWriter.h"

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

void WorldSerializer::saveToFile(const World& world,
				 const std::string& path) const {
	std::ofstream stream(path);
	if (!stream.is_open()) {
		throw FormatError(FormatErrorKind::UnwritableOutput, path);
	}
	stream << saveToString(world);
	stream.close();
	if (!stream) {
		throw FormatError(FormatErrorKind::UnwritableOutput, path);
	}
}

std::string WorldSerializer::saveToString(const World& world) const {
	return WorldTomlWriter().write(world);
}

}
