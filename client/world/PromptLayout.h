#pragma once

#include <map>
#include <string>
#include <vector>

#include "world/AnchorLayout.h"
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
// y relleno) y cómo se dibuja en el panel. Datos de [prompt.<botón>] en
// ui/text.toml (hoy cancel y ok): un botón nuevo es otra sección.
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
	// Dónde cae dentro del panel (margin se cuenta desde ese borde).
	Anchor anchor = Anchor::BottomRight;
};

// El copy de un popup: el texto que muestra y, según el tipo, sus
// opciones o el botón del panel. Todo es presentación (ui/text.toml).
struct PromptCopy {
	std::string text;
	// Vacío = no es una pregunta (no dibuja opciones).
	std::vector<std::string> options;
	// Nombre del botón ([prompt.<botón>]) o vacío = sin botón.
	std::string button;
	// Cuántos px baja la fila de opciones respecto de la pregunta
	// (0 = la separación normal de `line_gap`).
	int optionsOffsetY = 0;
};

// Qué popup pide cada copia. Sumar un popup = un valor acá + su sección
// [prompt.copy.<clave>] en ui/text.toml; el dibujo no cambia.
enum class PromptId {
	SleepQuestion,
	SleepWaiting,
	NextDay,
	GiftOffer,
	GiftWaiting,
	GiftReceived,
};

// El prompt del popup: copy, colores y layout (datos de la sección
// [prompt] de ui/text.toml, traducidos en su constructor, que lanza
// ConfigError) + la geometría de dónde cae cada cosa dentro del panel.
// Estilo InventoryLayout: datos y geometría juntos; el que dibuja
// (sdl::PromptPainter) solo pinta lo que esto calcula. Todo recibe la
// caja del panel: sirve para la grilla que sea.
struct PromptLayout {
	Color textColor;
	Color hoverColor;
	int textSize = 0;
	int lineGap = 0;

	// La forma del panel en slots [columnas, filas] (hoy 9x2; un
	// panel 9x1 o 3x4 es solo otro dato).
	GridSpan grid{0, 0};

	// Los botones del popup por nombre (kPromptCancelButton,
	// kPromptOkButton): mismo 9-slice, distinto dato.
	std::map<std::string, ButtonFrame> buttons;

	// El copy de cada popup, por identidad. Un popup nuevo no agrega
	// campos a esta clase: agrega una entrada acá (la lee el reader).
	std::map<PromptId, PromptCopy> prompts;

	explicit PromptLayout(const std::string& path);

	// Copy del popup pedido. El loader garantiza que estén todas las
	// entradas conocidas.
	const PromptCopy& prompt(PromptId id) const;

	// La pregunta arriba y la fila de opciones debajo: el bloque,
	// centrado verticalmente en el panel y con el ancho del panel (el
	// texto se centra adentro de cada caja).
	PixelBox questionBox(const PixelBox& panel) const;

	// Las opciones en UNA fila debajo de la pregunta, repartidas por
	// igual: una columna por opción (con dos: la primera a la
	// izquierda, la segunda a la derecha, a la misma altura). El copy
	// aporta cuántas son y su ajuste vertical.
	std::vector<PixelBox> optionBoxes(const PixelBox& panel,
					  const PromptCopy& copy) const;

	// El avance ("Esperando al resto..."), centrado en el panel.
	PixelBox waitingBox(const PixelBox& panel) const;

	// La pregunta del cambio de día: arriba del botón, con el ancho del
	// panel (el texto se centra adentro).
	PixelBox nextDayBox(const PixelBox& panel) const;

	// Índice de la opción bajo el punto (-1 si no hay ninguna): las
	// MISMAS cajas que se dibujan, así lo que se toca es lo que se ve.
	int optionAt(const PixelBox& panel, const PixelPoint& point,
		     const PromptCopy& copy) const;

	// Un botón (por nombre): su caja en el panel según su anchor, si el
	// punto la pega, y sus piezas 9-slice estiradas a esa caja.
	PixelBox buttonBox(const std::string& name, const PixelBox& panel) const;
	bool buttonAt(const std::string& name, const PixelBox& panel,
		      const PixelPoint& point) const;
	std::vector<FrameLayout::Piece> buttonPieces(
		const std::string& name, const PixelBox& button) const;

private:
	// Traduce y valida una pieza del 9-slice (`[col, row, cols, rows]`);
	// lanza ConfigError con el path completo de la clave.
	ButtonPiece readPiece(const tomlcfg::TomlDocument& doc,
			      const std::string& path) const;

	// Traduce y valida la sección [prompt.<name>] de un botón.
	void readButton(const tomlcfg::TomlDocument& doc,
			const std::string& name);

	// Traduce y valida una copia [prompt.copy.<key>].
	void readPrompt(const tomlcfg::TomlDocument& doc, PromptId id,
			const std::string& key);

	bool contains(const PixelBox& box, const PixelPoint& point) const;
};

}  // namespace world
