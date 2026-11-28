#include "world/TomlConfigReader.h"

#include <array>
#include <utility>

#include "world/ConfigError.h"

namespace world {

TomlConfigReader::TomlConfigReader(tomlcfg::TomlDocument doc,
				   std::string prefix)
	: doc_(std::move(doc)), prefix_(std::move(prefix)) {}

std::string TomlConfigReader::pathFor(const std::string& key) const {
	if (prefix_.empty()) {
		return key;
	}
	return prefix_ + "." + key;
}

bool TomlConfigReader::hasKey(const std::string& key) const {
	return doc_.hasKey(key);
}

TomlConfigReader TomlConfigReader::subtable(const std::string& key) const {
	if (!doc_.hasKey(key)) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  pathFor(key));
	}
	return TomlConfigReader(doc_.getSubtable(key), pathFor(key));
}

std::vector<TomlConfigReader> TomlConfigReader::tableArray(
	const std::string& key) const {
	const std::vector<tomlcfg::TomlDocument> entries =
		doc_.getTableArray(key);
	const std::string arrayPath = pathFor(key);
	if (entries.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure, arrayPath);
	}
	std::vector<TomlConfigReader> readers;
	readers.reserve(entries.size());
	for (std::size_t i = 0; i < entries.size(); ++i) {
		readers.emplace_back(
			entries[i], arrayPath + "[" + std::to_string(i) + "]");
	}
	return readers;
}

int TomlConfigReader::positiveInt(const std::string& key) const {
	const int value = doc_.getInt(key);
	if (value <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, pathFor(key));
	}
	return value;
}

int TomlConfigReader::nonNegativeInt(const std::string& key) const {
	const int value = doc_.getInt(key);
	if (value < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, pathFor(key));
	}
	return value;
}

int TomlConfigReader::intOr(const std::string& key, const int fallback) const {
	return doc_.hasKey(key) ? doc_.getInt(key) : fallback;
}

bool TomlConfigReader::boolOr(const std::string& key,
			      const bool fallback) const {
	return doc_.hasKey(key) ? doc_.getBool(key) : fallback;
}

double TomlConfigReader::fraction(const std::string& key) const {
	const double value = doc_.getDouble(key);
	if (value <= 0.0 || value > 1.0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, pathFor(key));
	}
	return value;
}

PixelSize TomlConfigReader::positiveSize(const std::string& key) const {
	const std::array<int, 2> size = doc_.getIntArray<2>(key);
	if (size[0] <= 0 || size[1] <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, pathFor(key));
	}
	return PixelSize{size[0], size[1]};
}

std::string TomlConfigReader::nonEmptyString(const std::string& key) const {
	std::string value = doc_.getString(key);
	if (value.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, pathFor(key));
	}
	return value;
}

Color TomlConfigReader::color(const std::string& key) const {
	const std::array<int, 4> rgba = doc_.getIntArray<4>(key);
	for (const int channel : rgba) {
		if (channel < 0 || channel > 255) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  pathFor(key));
		}
	}
	return Color{rgba[0], rgba[1], rgba[2], rgba[3]};
}

std::vector<std::string> TomlConfigReader::stringArray(
	const std::string& key) const {
	return doc_.getStringArray(key);
}

const tomlcfg::TomlDocument& TomlConfigReader::document() const {
	return doc_;
}

}  // namespace world
