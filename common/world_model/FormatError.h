#pragma once

#include <exception>
#include <string>

#include "toml/TomlDocument.h"

namespace worldmodel {

enum class FormatErrorKind {
	UnreadableInput,
	MissingStructure,
	WrongType,
	InvalidValue,
	UnwritableOutput,
};

class FormatError : public std::exception {
public:
	FormatError(FormatErrorKind kind, std::string path);
	explicit FormatError(const tomlcfg::TomlError& error);

	FormatErrorKind kind() const;
	const std::string& path() const;

	const char* what() const noexcept override;

private:
	FormatErrorKind kindFor(tomlcfg::TomlErrorKind tomlKind) const;

	FormatErrorKind kind_;
	std::string path_;
};

}
