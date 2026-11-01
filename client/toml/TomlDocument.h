#pragma once

#include <array>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

namespace tomlcfg {

enum class TomlErrorKind {
	MissingKey,
	WrongType,
	ParseError,
};

class TomlError : public std::exception {
public:
	TomlError(TomlErrorKind kind, const std::string& path);

	TomlErrorKind kind() const;
	const std::string& path() const;

	const char* what() const noexcept override;

private:
	TomlErrorKind kind_;
	std::string path_;
};

class TomlDocument {
public:
	enum class Source {
		File,
		Text,
	};

	TomlDocument(const std::string& content, Source source);

	TomlDocument(const TomlDocument& other);
	TomlDocument& operator=(const TomlDocument& other);
	TomlDocument(TomlDocument&& other) noexcept;
	TomlDocument& operator=(TomlDocument&& other) noexcept;
	~TomlDocument();

	bool hasKey(const std::string& path) const;

	int getInt(const std::string& path) const;
	std::string getString(const std::string& path) const;

	// Array de strings.
	std::vector<std::string> getStringArray(const std::string& path) const;

	double getDouble(const std::string& path) const;

	bool getBool(const std::string& path) const;

	template <std::size_t N>
	std::array<int, N> getIntArray(const std::string& path) const {
		std::array<int, N> result{};
		readIntArray(path, result.data(), N);
		return result;
	}

	std::vector<std::string> subtableKeys(const std::string& path) const;

	std::size_t arraySize(const std::string& path) const;

	TomlDocument getSubtable(const std::string& path) const;

	std::vector<TomlDocument> getTableArray(
		const std::string& path) const;

private:
	class Impl;

	explicit TomlDocument(Impl impl);

	void readIntArray(const std::string& path, int* out,
			  std::size_t count) const;

	Impl* impl_;
};

}
