#include "world/ConfigError.h"

#include <map>

namespace world {

ConfigError::ConfigError(ConfigErrorKind kind, const std::string& path)
	: kind_(kind), path_(path) {}

ConfigError::ConfigError(const tomlcfg::TomlError& error)
	: kind_(kindFor(error.kind())), path_(error.path()) {}

ConfigErrorKind ConfigError::kind() const { return kind_; }

const char* ConfigError::what() const noexcept { return path_.c_str(); }

const std::string& ConfigError::path() const { return path_; }

ConfigErrorKind ConfigError::kindFor(tomlcfg::TomlErrorKind tomlKind) const {
	const std::map<tomlcfg::TomlErrorKind, ConfigErrorKind> byKind = {
		{tomlcfg::TomlErrorKind::MissingKey,
		 ConfigErrorKind::MissingStructure},
		{tomlcfg::TomlErrorKind::WrongType, ConfigErrorKind::WrongType},
		{tomlcfg::TomlErrorKind::ParseError,
		 ConfigErrorKind::UnreadableInput},
	};
	const auto it = byKind.find(tomlKind);
	return it != byKind.end() ? it->second
				  : ConfigErrorKind::UnreadableInput;
}

}
