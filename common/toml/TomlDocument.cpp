#include "toml/TomlDocument.h"

#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include <toml++/toml.hpp>

namespace tomlcfg {


TomlError::TomlError(TomlErrorKind kind, const std::string& path)
	: kind_(kind), path_(path) {}

TomlErrorKind TomlError::kind() const { return kind_; }
const std::string& TomlError::path() const { return path_; }

const char* TomlError::what() const noexcept { return path_.c_str(); }


class TomlDocument::Impl {
public:
	toml::table table;
};

namespace {

const toml::node* find(const toml::table& root, const std::string& path) {
	return root.at_path(path).node();
}

}


TomlDocument::TomlDocument(Impl impl) : impl_(new Impl(std::move(impl))) {}

TomlDocument::TomlDocument(const TomlDocument& other)
	: impl_(new Impl(*other.impl_)) {}

TomlDocument& TomlDocument::operator=(const TomlDocument& other) {
	if (this != &other) {
		Impl* copy = new Impl(*other.impl_);
		delete impl_;
		impl_ = copy;
	}
	return *this;
}

TomlDocument::TomlDocument(TomlDocument&& other) noexcept
	: impl_(other.impl_) {
	other.impl_ = nullptr;
}

TomlDocument& TomlDocument::operator=(TomlDocument&& other) noexcept {
	if (this != &other) {
		delete impl_;
		impl_ = other.impl_;
		other.impl_ = nullptr;
	}
	return *this;
}

TomlDocument::~TomlDocument() { delete impl_; }


namespace {

toml::table parseContent(const std::string& content,
			 TomlDocument::Source source) {
	try {
		if (source == TomlDocument::Source::File) {
			return ::toml::parse_file(content);
		}
		return ::toml::parse(content);
	} catch (const ::toml::parse_error& error) {
		const std::string where =
			source == TomlDocument::Source::File
				? content + " (" +
					  std::string(error.description()) + ")"
				: std::string(error.description());
		throw TomlError(TomlErrorKind::ParseError, where);
	}
}

}

TomlDocument::TomlDocument(const std::string& content, Source source)
	: impl_(new Impl{parseContent(content, source)}) {}


bool TomlDocument::hasKey(const std::string& path) const {
	return find(impl_->table, path) != nullptr;
}

int TomlDocument::getInt(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const std::optional<int64_t> value = node->value<int64_t>();
	if (!value.has_value()) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	return static_cast<int>(*value);
}

double TomlDocument::getDouble(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const std::optional<double> asDouble = node->value<double>();
	if (asDouble.has_value()) {
		return *asDouble;
	}
	const std::optional<int64_t> asInt = node->value<int64_t>();
	if (asInt.has_value()) {
		return static_cast<double>(*asInt);
	}
	throw TomlError(TomlErrorKind::WrongType, path);
}

std::string TomlDocument::getString(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const std::optional<std::string> value = node->value<std::string>();
	if (!value.has_value()) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	return *value;
}

std::vector<std::string> TomlDocument::getStringArray(
	const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::array* array = node->as_array();
	if (array == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	std::vector<std::string> values;
	for (const toml::node& element : *array) {
		const std::optional<std::string> value =
			element.value<std::string>();
		if (!value.has_value()) {
			throw TomlError(TomlErrorKind::WrongType, path);
		}
		values.push_back(std::move(*value));
	}
	return values;
}

bool TomlDocument::getBool(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const std::optional<bool> value = node->value<bool>();
	if (!value.has_value()) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	return *value;
}

std::vector<int> TomlDocument::getIntArray(const std::string& path) const {
	std::vector<int> values(arraySize(path));
	readIntArray(path, values.data(), values.size());
	return values;
}

std::vector<bool> TomlDocument::getBoolArray(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::array* array = node->as_array();
	if (array == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	std::vector<bool> values;
	values.reserve(array->size());
	for (const toml::node& element : *array) {
		// as_boolean y no value<bool>: toml++ acepta 1/0 como bool.
		const toml::value<bool>* value = element.as_boolean();
		if (value == nullptr) {
			throw TomlError(TomlErrorKind::WrongType, path);
		}
		values.push_back(value->get());
	}
	return values;
}

void TomlDocument::readIntArray(const std::string& path, int* out,
				std::size_t count) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::array* array = node->as_array();
	if (array == nullptr || array->size() != count) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	for (std::size_t index = 0; index < count; ++index) {
		const std::optional<int64_t> value =
			(*array)[index].value<int64_t>();
		if (!value.has_value() ||
			*value < std::numeric_limits<int>::min() ||
			*value > std::numeric_limits<int>::max()) {
			throw TomlError(TomlErrorKind::WrongType, path);
		}
		out[index] = static_cast<int>(*value);
	}
}

std::size_t TomlDocument::arraySize(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::array* array = node->as_array();
	if (array == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	return array->size();
}

std::vector<std::string> TomlDocument::subtableKeys(
	const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::table* table = node->as_table();
	if (table == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	std::vector<std::string> keys;
	for (const auto& [key, value] : *table) {
		(void)value;
		keys.emplace_back(key.str());
	}
	return keys;
}

TomlDocument TomlDocument::getSubtable(const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::table* table = node->as_table();
	if (table == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	Impl impl;
	impl.table = *table;
	return TomlDocument(std::move(impl));
}

std::vector<TomlDocument> TomlDocument::getTableArray(
	const std::string& path) const {
	const toml::node* node = find(impl_->table, path);
	if (node == nullptr) {
		throw TomlError(TomlErrorKind::MissingKey, path);
	}
	const toml::array* array = node->as_array();
	if (array == nullptr) {
		throw TomlError(TomlErrorKind::WrongType, path);
	}
	std::vector<TomlDocument> documents;
	for (const toml::node& element : *array) {
		const toml::table* table = element.as_table();
		if (table == nullptr) {
			throw TomlError(TomlErrorKind::WrongType, path);
		}
		Impl impl;
		impl.table = *table;
		documents.push_back(TomlDocument(std::move(impl)));
	}
	return documents;
}

}
