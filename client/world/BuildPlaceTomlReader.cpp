#include "world/BuildPlaceTomlReader.h"

#include <array>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kSheetImage = "sheet.image";
constexpr const char* kSheetCell = "sheet.cell";
constexpr const char* kStates = "states";
constexpr const char* kBlock = "block";
constexpr const char* kScale = "scale";

// Estado lógico -> tabla del TOML que lo describe.
struct StateBinding {
	BuildPlaceState state;
	const char* tableKey;
};

constexpr std::array<StateBinding, 2> kStateBindings = {{
	{BuildPlaceState::Available, "available"},
	{BuildPlaceState::Placed, "placed"},
}};

}

BuildPlaceTomlReader::BuildPlaceTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string BuildPlaceTomlReader::join(const std::string& base,
				       const std::string& key) const {
	return base + "." + key;
}

const BuildPlaceState* BuildPlaceTomlReader::stateFor(
	const std::string& name) const {
	for (const StateBinding& binding : kStateBindings) {
		if (name == binding.tableKey) {
			return &binding.state;
		}
	}
	return nullptr;
}

BuildPlaceVisual BuildPlaceTomlReader::readVisual(const std::string& sheet,
						  int cellWidth,
						  int cellHeight,
						  const std::string& base) const {
	// Bloque [col, row, cols, rows] en celdas de la hoja: el cuadrado
	// completo (composición fija), con su esquina sup-izq en sprite.
	const std::string blockPath = join(base, kBlock);
	const std::array<int, 4> block = doc_.getIntArray<4>(blockPath);
	if (block[0] < 0 || block[1] < 0 || block[2] <= 0 || block[3] <= 0) {
		throw ConfigError(Kind::InvalidValue, blockPath);
	}

	BuildPlaceVisual visual;
	visual.sprite.sheetImage = sheet;
	visual.sprite.cellWidth = cellWidth;
	visual.sprite.cellHeight = cellHeight;
	visual.sprite.col = block[0];
	visual.sprite.row = block[1];
	visual.cols = block[2];
	visual.rows = block[3];

	const std::string scalePath = join(base, kScale);
	if (doc_.hasKey(scalePath)) {
		visual.scale = doc_.getDouble(scalePath);
		if (visual.scale <= 0.0) {
			throw ConfigError(Kind::InvalidValue, scalePath);
		}
	}
	return visual;
}

BuildPlaceCatalog BuildPlaceTomlReader::read() const {
	const std::string sheetPath = kSheetImage;
	const std::string sheet = doc_.getString(sheetPath);
	if (sheet.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string cellPath = kSheetCell;
	const std::array<int, 2> cell = doc_.getIntArray<2>(cellPath);
	if (cell[0] <= 0 || cell[1] <= 0) {
		throw ConfigError(Kind::InvalidValue, cellPath);
	}

	// Cada subtabla de states es un estado por su nombre (el
	// vocabulario de world::BuildPlaceState). Lo que no se conoce es un
	// error de configuración, no se ignora en silencio.
	BuildPlaceCatalog catalog;
	for (const std::string& name : doc_.subtableKeys(kStates)) {
		const BuildPlaceState* state = stateFor(name);
		if (state == nullptr) {
			throw ConfigError(Kind::InvalidValue,
					  join(kStates, name));
		}
		catalog.setState(*state, readVisual(sheet, cell[0], cell[1],
						   join(kStates, name)));
	}
	return catalog;
}

}
