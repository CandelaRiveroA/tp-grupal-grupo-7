#pragma once

#include <string>

#include "toml/TomlDocument.h"
#include "world_model/FormatError.h"

namespace worldmodel {

// Parte común de los serializers: parsea con el reader elegido y escribe
// texto a disco reportando UnwritableOutput.
class SerializerIO {
public:
	template <typename Value, typename Reader>
	static Value load(const std::string& content,
			  tomlcfg::TomlDocument::Source source) {
		try {
			const tomlcfg::TomlDocument doc(content, source);
			return Reader(doc).read();
		} catch (const tomlcfg::TomlError& error) {
			// Los FormatError semánticos se propagan tal cual.
			throw FormatError(error);
		}
	}

	static void save(const std::string& text, const std::string& path);
};

}
