#pragma once

#include <optional>
#include <string>

#include "world/AnchorLayout.h"
#include "world/Color.h"
#include "world/PixelBox.h"

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
// agrega sus datos al TOML. cols/rows = celdas de la hoja que abarca
// (`span`, default 1x1: los muebles ocupan varias). drawn = tamaño en el
// mapa (0 = el nativo: celda * cols/rows); offset_y = ajuste vertical al apoyarlo en el
// piso (0 = pegado a los pies; para hojas con aire transparente
// abajo). anchor = en qué esquina de su huella cae la celda que manda la
// fuente de estado (solo los muebles lo usan). La hoja se resuelve por
// SheetCache al dibujar.
struct SpriteVisual {
	std::string sheetImage;
	int col = 0;
	int row = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	// Bloque de celdas que dibuja el sprite ([col, row] es su esquina
	// sup-izq): el mismo concepto [col, row, cols, rows] de las capas de
	// los overlays. 1x1 = una celda (el caso histórico). En el TOML, la
	// key opcional `span = [cols, rows]`.
	int cols = 1;
	int rows = 1;
	int drawnWidth = 0;
	int drawnHeight = 0;
	// Desplazamiento opcional en px de mundo (offset_x / offset_y, default 0).
	int offsetX = 0;
	int offsetY = 0;
	// En qué esquina de su huella cae la celda que manda la fuente de
	// estado (solo los muebles lo usan). bottom_left (default) = el
	// sprite crece hacia la derecha y hacia arriba desde esa celda;
	// top_left = esa celda es la superior y el sprite crece hacia abajo.
	// El vocabulario es el de AnchorLayout, uno solo para todos los
	// lectores.
	Anchor anchor = Anchor::BottomLeft;
	// Tinte opcional (color modulation): los sprites claros de la hoja
	// se dibujan multiplicados por este color. Sin tinte = tal cual.
	std::optional<Color> tint;

	SpriteVisual() = default;

	// Lee sheet/cell/span/drawn de la subtabla `base`, SIN celda de posición:
	// para hojas cuyas celdas las da otro dato (ej. los frames de un
	// diálogo, cada uno en su propia celda).
	SpriteVisual(const tomlcfg::TomlDocument& doc, const std::string& base);

	// Lee el sprite completo de la subtabla `base`: `<base>.sheet`,
	// `<base>.cell`, `<base>.<posKey>` ([col, row]) y `<base>.drawn`
	// (opcional; 0 = nativo). `<base>.span` = [cols, rows] (opcional,
	// ambos > 0; default 1x1). Cada formato TOML nombra distinto a la
	// celda de posición ("at", "icon", ...), por eso `posKey`.
	// Validado: lanza ConfigError(InvalidValue, clave exacta) con la
	// clave que falló.
	SpriteVisual(const tomlcfg::TomlDocument& doc, const std::string& base,
		     const std::string& posKey);

	// Tamaño final en pantalla: `drawn` si lo declara, si no el nativo
	// de la región (celda * cols/rows). Todos los que dibujan un sprite usan
	// esta regla (ítems, cursores, diálogos).
	int displayWidth() const;
	int displayHeight() const;

	// Caja centrada dentro de `box` que conserva la proporción del
	// tamaño en pantalla (el sprite entra sin deformarse). Si el
	// tamaño no es válido devuelve `box`.
	PixelBox fitIn(const PixelBox& box) const;
};

// Región rectangular de una hoja: la celda de origen (arriba-izquierda) y
// cuántas celdas abarca en cada eje. Es el mismo concepto [col, row, cols,
// rows] que usan las capas de los overlays (tools_overlay.toml) y el
// dibujado de bloques (SpriteSheet::drawBlock). Un sprite de una sola
// celda es cols=rows=1.
struct SheetRegion {
	SheetCell at;
	int cols = 1;
	int rows = 1;

	SheetRegion() = default;

	// Región por valor (para construir desde datos ya leídos).
	SheetRegion(const SheetCell& topLeft, int colsValue, int rowsValue);
};

}
