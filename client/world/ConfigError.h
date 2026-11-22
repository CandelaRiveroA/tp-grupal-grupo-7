#pragma once

#include <exception>
#include <string>

#include "toml/TomlDocument.h"
#include "world_model/FormatError.h"

namespace world {

enum class ConfigErrorKind {
	UnreadableInput,
	MissingStructure,
	WrongType,
	InvalidValue,
};

class ConfigError : public std::exception {
public:
	ConfigError(ConfigErrorKind kind, const std::string& path);

	explicit ConfigError(const tomlcfg::TomlError& error);

	// Traduce el error del modelo de mundo del editor (common/
	// world_model): misma categoría y mismo path crudo.
	explicit ConfigError(const worldmodel::FormatError& error);

	ConfigErrorKind kind() const;

	const char* what() const noexcept override;

	const std::string& path() const;

private:
	ConfigErrorKind kindFor(tomlcfg::TomlErrorKind tomlKind) const;
	ConfigErrorKind kindFor(worldmodel::FormatErrorKind formatKind) const;

	ConfigErrorKind kind_;
	std::string path_;
};

}
