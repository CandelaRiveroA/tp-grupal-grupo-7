#pragma once

#include <string>

#include "world/Color.h"

namespace world {

// Datos de presentación del chatbox (§5): medidas fijas del TOML; los
// altos salen del contenido. El reader llena TODAS las keys (lanza
// ConfigError si falta alguna), así que no hay valores de default.
struct ChatLayout {
	// Hoja de sprites de la caja (relativa a assets/) y su celda en px.
	std::string sheetImage;
	int sheetCellWidth;
	int sheetCellHeight;

	// Grosor del marco: esquinas fijas 1:1, bordes y centro estirados.
	// 0 = sin 9-slice (sprite entero estirado).
	int sheetBorder;

	// Ancho de las cajas como fracción del ancho de la ventana (0..1).
	double widthFraction;

	// Margen izquierdo y límite de abajo del chat (px).
	int marginX;
	int marginBottom;

	// Altura de la línea de tipeo como fracción de la ventana desde
	// arriba; el historial se apila arriba suyo.
	double inputFraction;

	int textPad;
	int fontSize;
	Color textColor;

	// Preview del chat cerrado: alpha de su TEXTO y renglones que se ven.
	int previewAlpha;
	int previewRows;

	int lineHeight() const { return fontSize + textPad; }
};

}