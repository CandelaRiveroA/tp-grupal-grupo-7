#pragma once

#include <string>

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Celda (col,row) de una hoja de sprites. Unifica lo que antes eran
// tres structs iguales: BannerCell, DigitCell y FrameIndex.
struct SheetCell {
	int col = 0;
	int row = 0;

	SheetCell() = default;

	// Celda por valor (para construir desde datos ya leídos).
	SheetCell(int colValue, int rowValue);

	// Lee el par [col, row] de la clave `key` del TOML (validado: >= 0).
	// Lanza ConfigError(InvalidValue, key) si no cumple.
	SheetCell(const tomlcfg::TomlDocument& doc, const std::string& key);
};

// Referencia visual de un sprite: qué hoja, qué celda y cómo se dibuja
// en el mundo. Es EL struct común de todo lo que se muestra (ítems,
// íconos, productos): una cosa nueva comparte este mecanismo y solo
// agrega sus datos al TOML. drawn = tamaño en el mapa (0 = el nativo de
// la celda de la hoja). La hoja se resuelve por SheetCache al dibujar.
struct SpriteVisual {
	std::string sheetImage;
	int col = 0;
	int row = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	int drawnWidth = 0;
	int drawnHeight = 0;

	SpriteVisual() = default;

	// Lee el sprite completo de la subtabla `base`: `<base>.sheet`,
	// `<base>.cell`, `<base>.<posKey>` ([col, row]) y `<base>.drawn`
	// (opcional; 0 = nativo). Cada formato TOML nombra distinto a la
	// celda de posición ("at", "icon", ...), por eso `posKey`.
	// Validado: lanza ConfigError(InvalidValue, clave exacta) con la
	// clave que falló.
	SpriteVisual(const tomlcfg::TomlDocument& doc, const std::string& base,
		     const std::string& posKey);
};

}
