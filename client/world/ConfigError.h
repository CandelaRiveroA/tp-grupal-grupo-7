#pragma once

#include <exception>
#include <string>

#include "toml/TomlDocument.h"

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

	ConfigErrorKind kind() const;

	const char* what() const noexcept override;

	const std::string& path() const;

private:
	ConfigErrorKind kindFor(tomlcfg::TomlErrorKind tomlKind) const;

	ConfigErrorKind kind_;
	std::string path_;
};

}
