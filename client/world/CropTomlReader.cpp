#include "world/CropTomlReader.h"

#include <utility>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kSheetImage = "sheet.image";
constexpr const char* kSheetCell = "sheet.cell";
constexpr const char* kPlantScale = "plant.scale";
constexpr const char* kHarvestScale = "plant.harvest_scale";
constexpr const char* kCrops = "crops";
constexpr const char* kSackId = "sack_id";
constexpr const char* kSack = "sack";
constexpr const char* kSeed = "seed";
constexpr const char* kGrowth = "growth";
constexpr const char* kQualities = "qualities";
constexpr const char* kDroppable = "droppable";

// Etapas fijas del ciclo de vida: 5 crecimientos y 5 calidades.
constexpr std::size_t kStageCount = 5;

}

CropTomlReader::CropTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string CropTomlReader::join(const std::string& base,
				 const std::string& key) const {
	return base + "." + key;
}

std::string CropTomlReader::idx(const std::string& base,
				std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

std::array<CropQuality, 5> CropTomlReader::qualityOrder() const {
	return {CropQuality::Base, CropQuality::Good, CropQuality::Excellent,
		CropQuality::Incredible, CropQuality::Perfect};
}

SpriteVisual CropTomlReader::readSprite(const std::string& sheet,
					int cellWidth, int cellHeight,
					const std::string& cellPath) const {
	const SheetCell cell(doc_, cellPath);
	SpriteVisual visual;
	visual.sheetImage = sheet;
	visual.cellWidth = cellWidth;
	visual.cellHeight = cellHeight;
	visual.col = cell.col;
	visual.row = cell.row;
	return visual;
}

std::vector<SpriteVisual> CropTomlReader::readStages(
	const std::string& sheet, int cellWidth, int cellHeight,
	const std::string& path) const {
	const std::size_t count = doc_.arraySize(path);
	if (count != kStageCount) {
		throw ConfigError(Kind::InvalidValue, path);
	}
	std::vector<SpriteVisual> stages;
	for (std::size_t i = 0; i < count; ++i) {
		stages.push_back(
			readSprite(sheet, cellWidth, cellHeight, idx(path, i)));
	}
	return stages;
}

CropDefinition CropTomlReader::readCrop(const std::string& base,
					const std::string& sheet,
					int cellWidth,
					int cellHeight) const {
	CropDefinition definition;
	const std::string sackIdPath = join(base, kSackId);
	definition.sackId = doc_.getString(sackIdPath);
	if (definition.sackId.empty()) {
		throw ConfigError(Kind::InvalidValue, sackIdPath);
	}
	definition.sack = readSprite(sheet, cellWidth, cellHeight,
				    join(base, kSack));
	definition.seed = readSprite(sheet, cellWidth, cellHeight,
				    join(base, kSeed));
	definition.growth = readStages(sheet, cellWidth, cellHeight,
				      join(base, kGrowth));

	// Calidades: las 5 celdas en el orden fijo del ciclo (base ->
	// perfecta) se indexan por la calidad que representan.
	const std::string qualitiesPath = join(base, kQualities);
	const std::vector<SpriteVisual> cells = readStages(
		sheet, cellWidth, cellHeight, qualitiesPath);
	const std::array<CropQuality, 5> order = qualityOrder();
	for (std::size_t i = 0; i < kStageCount; ++i) {
		definition.qualities.emplace(order[i], cells[i]);
	}

	const std::string droppablePath = join(base, kDroppable);
	definition.droppable = doc_.hasKey(droppablePath) &&
			       doc_.getBool(droppablePath);
	return definition;
}

CropCatalog CropTomlReader::read() const {
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

	CropCatalog catalog;

	const std::string scalePath = kPlantScale;
	const double scale = doc_.getDouble(scalePath);
	if (scale <= 0.0) {
		throw ConfigError(Kind::InvalidValue, scalePath);
	}
	catalog.setPlantScale(scale);

	const std::string harvestPath = kHarvestScale;
	const double harvestScale = doc_.getDouble(harvestPath);
	if (harvestScale <= 0.0) {
		throw ConfigError(Kind::InvalidValue, harvestPath);
	}
	catalog.setHarvestScale(harvestScale);

	for (const std::string& cropId : doc_.subtableKeys(kCrops)) {
		// cropId = nombre de la subtabla (contentId del PRODUCTO
		// cosechado). No se traduce a ningún enum: el inventario es
		// genérico.
		catalog.setEntry(cropId,
				 readCrop(join(kCrops, cropId), sheet, cell[0],
					  cell[1]));
	}
	return catalog;
}

}
