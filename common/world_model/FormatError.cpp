#include "world_model/FormatError.h"

#include <map>
#include <utility>

namespace worldmodel {

FormatError::FormatError(FormatErrorKind kind, std::string path)
	: kind_(kind), path_(std::move(path)) {}

FormatError::FormatError(const tomlcfg::TomlError& error)
	: kind_(kindFor(error.kind())), path_(error.path()) {}

FormatErrorKind FormatError::kind() const { return kind_; }

const std::string& FormatError::path() const { return path_; }

const char* FormatError::what() const noexcept { return path_.c_str(); }

FormatErrorKind FormatError::kindFor(tomlcfg::TomlErrorKind tomlKind) const {
	const std::map<tomlcfg::TomlErrorKind, FormatErrorKind> byKind = {
		{tomlcfg::TomlErrorKind::MissingKey,
		 FormatErrorKind::MissingStructure},
		{tomlcfg::TomlErrorKind::WrongType, FormatErrorKind::WrongType},
		{tomlcfg::TomlErrorKind::ParseError,
		 FormatErrorKind::UnreadableInput},
	};
	const auto it = byKind.find(tomlKind);
	return it != byKind.end() ? it->second
				  : FormatErrorKind::UnreadableInput;
}

}
