#include "world/GroundTomlReader.h"

#include <array>
#include <utility>

#include "world/ConfigError.h"
#include "world/WorldConstants.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kSheetImage = "sheet.image";
constexpr const char* kSheetCell = "sheet.cell";
constexpr const char* kGround = "ground";
constexpr const char* kSprite = "sprite";
constexpr const char* kTiles = "tiles";
// Hoja propia opcional de una consecuencia (con su tamaño de celda).
constexpr const char* kImage = "image";
constexpr const char* kCell = "cell";
// Opcionales del sprite de la consecuencia: bloque de celdas, tinte y
// tamaño en el mapa.
constexpr const char* kBlock = "block";
constexpr const char* kTint = "tint";
constexpr const char* kDrawn = "drawn";

// Consecuencia lógica -> tabla del TOML que la describe.
struct GroundBinding {
	GroundConsequence consequence;
	const char* tableKey;
};

constexpr std::array<GroundBinding, 3> kGroundBindings = {{
	{GroundConsequence::Dug, "dug"},
	{GroundConsequence::Watered, "watered"},
	{GroundConsequence::Blood, "blood"},
}};

// Pieza del auto-tiling -> máscara de vecinos que la corresponde. El
// nombre describe la POSICIÓN de la pieza dentro de la zona armada y la
// máscara son sus vecinos presentes en el parche (kNeighbor*): por
// ejemplo `h_left` es la punta izquierda de una fila horizontal (solo
// tiene vecino al este) y `corner_top_left` la esquina superior
// izquierda de una zona (vecinos al este y al sur). Sumar una pieza
// nueva = una entrada acá + su celda en el TOML (§4: tabla, no switch).
struct PieceBinding {
	const char* role;
	std::uint8_t mask;
};

constexpr std::array<PieceBinding, 16> kPieceBindings = {{
	{"isolated", 0},
	{"h_left", kNeighborEast},
	{"h_right", kNeighborWest},
	{"h_middle", kNeighborEast | kNeighborWest},
	{"v_top", kNeighborSouth},
	{"v_bottom", kNeighborNorth},
	{"v_middle", kNeighborNorth | kNeighborSouth},
	{"corner_top_left", kNeighborEast | kNeighborSouth},
	{"corner_top_right", kNeighborWest | kNeighborSouth},
	{"corner_bottom_left", kNeighborNorth | kNeighborEast},
	{"corner_bottom_right", kNeighborNorth | kNeighborWest},
	{"edge_top", kNeighborEast | kNeighborSouth | kNeighborWest},
	{"edge_bottom", kNeighborNorth | kNeighborEast | kNeighborWest},
	{"edge_left", kNeighborNorth | kNeighborSouth | kNeighborEast},
	{"edge_right", kNeighborNorth | kNeighborSouth | kNeighborWest},
	{"interior", kNeighborNorth | kNeighborEast | kNeighborSouth |
			     kNeighborWest},
}};

}

GroundTomlReader::GroundTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string GroundTomlReader::join(const std::string& base,
				   const std::string& key) const {
	return base + "." + key;
}

std::string GroundTomlReader::idx(const std::string& base,
				  const std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

const GroundConsequence* GroundTomlReader::consequenceFor(
	const std::string& name) const {
	for (const GroundBinding& binding : kGroundBindings) {
		if (name == binding.tableKey) {
			return &binding.consequence;
		}
	}
	return nullptr;
}

const std::uint8_t* GroundTomlReader::maskFor(const std::string& role) const {
	for (const PieceBinding& binding : kPieceBindings) {
		if (role == binding.role) {
			return &binding.mask;
		}
	}
	return nullptr;
}

SpriteVisual GroundTomlReader::readSprite(const std::string& sheet,
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

void GroundTomlReader::readSpriteExtras(const std::string& base,
					SpriteVisual& sprite) const {
	// block = [cols, rows]: el sprite es un BLOQUE de celdas (la
	// salpicadura de la sangre arma sus gotas en una región 4x3).
	// Opcional; sin la clave, una celda.
	const std::string blockPath = join(base, kBlock);
	if (doc_.hasKey(blockPath)) {
		const std::array<int, 2> block =
			doc_.getIntArray<2>(blockPath);
		if (block[0] <= 0 || block[1] <= 0) {
			throw ConfigError(Kind::InvalidValue, blockPath);
		}
		sprite.cols = block[0];
		sprite.rows = block[1];
	}
	// tint = [r, g, b]: recoloreo del sprite (los blancos de la hoja
	// de la sangre salen bordó). Opcional.
	const std::string tintPath = join(base, kTint);
	if (doc_.hasKey(tintPath)) {
		const std::array<int, 3> tint = doc_.getIntArray<3>(tintPath);
		if (tint[0] < 0 || tint[0] > 255 || tint[1] < 0 ||
		    tint[1] > 255 || tint[2] < 0 || tint[2] > 255) {
			throw ConfigError(Kind::InvalidValue, tintPath);
		}
		sprite.tint = Color{tint[0], tint[1], tint[2], 255};
	}
	// drawn = [w, h]: tamaño en el mapa (sin la clave, el sprite cubre
	// la celda entera). Opcional.
	const std::string drawnPath = join(base, kDrawn);
	if (doc_.hasKey(drawnPath)) {
		const std::array<int, 2> drawn =
			doc_.getIntArray<2>(drawnPath);
		if (drawn[0] <= 0 || drawn[1] <= 0) {
			throw ConfigError(Kind::InvalidValue, drawnPath);
		}
		sprite.drawnWidth = drawn[0];
		sprite.drawnHeight = drawn[1];
	}
}

void GroundTomlReader::readPieces(const std::string& tilesPath,
				  const std::string& sheet, int cellWidth,
				  int cellHeight,
				  GroundVisual& visual) const {
	if (!doc_.hasKey(tilesPath)) {
		return;
	}
	// Cada clave es una pieza por su nombre (el vocabulario de
	// kPieceBindings); lo que no se conoce es error, no se ignora.
	// Cada valor es la lista de sus variantes dibujadas (1 o más).
	for (const std::string& role : doc_.subtableKeys(tilesPath)) {
		const std::uint8_t* mask = maskFor(role);
		const std::string rolePath = join(tilesPath, role);
		if (mask == nullptr) {
			throw ConfigError(Kind::InvalidValue, rolePath);
		}
		const std::size_t count = doc_.arraySize(rolePath);
		if (count == 0) {
			throw ConfigError(Kind::InvalidValue, rolePath);
		}
		std::vector<SpriteVisual> variants;
		for (std::size_t i = 0; i < count; ++i) {
			variants.push_back(readSprite(sheet, cellWidth, cellHeight,
						     idx(rolePath, i)));
		}
		visual.pieces.emplace(*mask, std::move(variants));
	}
}

GroundCatalog GroundTomlReader::read() const {
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

	// Cada subtabla de ground es una consecuencia por su nombre (el
	// vocabulario de world::GroundConsequence). Lo que no se conoce es
	// un error de configuración, no se ignora en silencio.
	GroundCatalog catalog;
	for (const std::string& name : doc_.subtableKeys(kGround)) {
		const GroundConsequence* consequence = consequenceFor(name);
		if (consequence == nullptr) {
			throw ConfigError(Kind::InvalidValue, join(kGround, name));
		}
		const std::string base = join(kGround, name);
		// Hoja propia OPCIONAL de la consecuencia (la sangre vive en
		// Animations.png, no en hoeDirt): `image` + `cell` la
		// declaran; sin ellas se usa la hoja global.
		std::string consequenceSheet = sheet;
		int cellWidth = cell[0];
		int cellHeight = cell[1];
		const std::string imagePath = join(base, kImage);
		if (doc_.hasKey(imagePath)) {
			consequenceSheet = doc_.getString(imagePath);
			if (consequenceSheet.empty()) {
				throw ConfigError(Kind::InvalidValue, imagePath);
			}
			const std::string cellPath = join(base, kCell);
			const std::array<int, 2> ownCell =
				doc_.getIntArray<2>(cellPath);
			if (ownCell[0] <= 0 || ownCell[1] <= 0) {
				throw ConfigError(Kind::InvalidValue, cellPath);
			}
			cellWidth = ownCell[0];
			cellHeight = ownCell[1];
		}
		GroundVisual visual;
		visual.sprite = readSprite(consequenceSheet, cellWidth,
					   cellHeight, join(base, kSprite));
		readSpriteExtras(base, visual.sprite);
		readPieces(join(base, kTiles), consequenceSheet, cellWidth,
			   cellHeight, visual);
		catalog.setConsequence(*consequence, std::move(visual));
	}
	return catalog;
}

}
