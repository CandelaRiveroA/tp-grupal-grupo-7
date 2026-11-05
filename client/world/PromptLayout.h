#pragma once

#include <string>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/InventoryLayout.h"
#include "world/ScenePixelMapper.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Una pieza de un 9-slice: la celda de hoja de su origen y su tamaño en
// celdas.
struct ButtonPiece {
	SheetCell cell;
	int srcCols = 0;
	int srcRows = 0;
};

// El marco 9-slice de un botón: su hoja, las 9 piezas (esquinas, bordes
// y relleno) y cómo se dibuja en el panel. Datos de [prompt.cancel] en
// ui/text.toml.
struct ButtonFrame {
	std::string sheet;
	int cellWidth = 0;
	int cellHeight = 0;
	ButtonPiece topLeft, top, topRight;
	ButtonPiece left, fill, right;
	ButtonPiece bottomLeft, bottom, bottomRight;
	int border = 0;
	PixelSize size;
	PixelSize margin;
};

// El prompt del popup ("¿Terminamos el día?"): copy, colores y layout
// (datos de la sección [prompt] de ui/text.toml, traducidos en su
// constructor, que lanza ConfigError) + la geometría de dónde cae cada
// cosa dentro del panel. Estilo InventoryLayout: datos y geometría
// juntos; el que dibuja (sdl::PromptPainter) solo pinta lo que esto
// calcula. Todo recibe la caja del panel: sirve para la grilla que sea.
struct PromptLayout {
	std::string question;
	std::vector<std::string> options;
	std::string waiting;
	Color textColor;
	Color hoverColor;
	int textSize = 0;
	int lineGap = 0;

	// La forma del panel en slots [columnas, filas] (hoy la barra:
	// 9x1; un panel 2x9 o 3x4 es solo otro dato).
	GridSpan grid{0, 0};

	ButtonFrame cancel;

	explicit PromptLayout(const std::string& path);

	// La pregunta arriba y la fila de opciones debajo: el bloque,
	// centrado verticalmente en el panel y con el ancho del panel (el
	// texto se centra adentro de cada caja).
	PixelBox questionBox(const PixelBox& panel) const;

	// Las opciones en UNA fila debajo de la pregunta, repartidas por
	// igual: una columna por opción (con dos: la primera a la
	// izquierda, la segunda a la derecha, a la misma altura).
	std::vector<PixelBox> optionBoxes(const PixelBox& panel) const;

	// El avance ("Esperando al resto..."), centrado en el panel.
	PixelBox waitingBox(const PixelBox& panel) const;

	// Índice de la opción bajo el punto (-1 si no hay ninguna): las
	// MISMAS cajas que se dibujan, así lo que se toca es lo que se ve.
	int optionAt(const PixelBox& panel, const PixelPoint& point) const;

	// El botón de cancelar: su caja abajo a la derecha del panel, si el
	// punto la pega, y sus piezas 9-slice estiradas a esa caja.
	PixelBox cancelBox(const PixelBox& panel) const;
	bool cancelAt(const PixelBox& panel, const PixelPoint& point) const;
	std::vector<FrameLayout::Piece> cancelPieces(
		const PixelBox& button) const;

private:
	// Traduce y valida una pieza del 9-slice (`[col, row, cols, rows]`);
	// lanza ConfigError con el path completo de la clave.
	ButtonPiece readPiece(const tomlcfg::TomlDocument& doc,
			      const std::string& path) const;

	// Traduce y valida la sección [prompt.cancel].
	void readCancel(const tomlcfg::TomlDocument& doc);

	bool contains(const PixelBox& box, const PixelPoint& point) const;
};

}  // namespace world
