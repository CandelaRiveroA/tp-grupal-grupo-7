#pragma once

#include <string>
#include <vector>

#include "toml/TomlDocument.h"
#include "world/Color.h"
#include "world/PixelBox.h"

namespace world {

// Lector de configuración TOML con prefijo de path acumulado: centraliza las
// validaciones repetidas (enteros positivos/no negativos, fracciones, tamaños,
// strings no vacíos, colores RGBA, subtablas y arrays) y garantiza paths
// completos en ConfigError (convenciones.md §3).
class TomlConfigReader {
public:
	explicit TomlConfigReader(tomlcfg::TomlDocument doc,
				  std::string prefix = {});

	std::string pathFor(const std::string& key) const;
	bool hasKey(const std::string& key) const;

	TomlConfigReader subtable(const std::string& key) const;
	std::vector<TomlConfigReader> tableArray(const std::string& key) const;

	int positiveInt(const std::string& key) const;
	int nonNegativeInt(const std::string& key) const;
	int intOr(const std::string& key, int fallback) const;
	bool boolOr(const std::string& key, bool fallback) const;
	double fraction(const std::string& key) const;
	PixelSize positiveSize(const std::string& key) const;
	std::string nonEmptyString(const std::string& key) const;
	Color color(const std::string& key) const;
	std::vector<std::string> stringArray(const std::string& key) const;

	const tomlcfg::TomlDocument& document() const;

private:
	tomlcfg::TomlDocument doc_;
	std::string prefix_;
};

}  // namespace world
