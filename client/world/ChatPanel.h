#pragma once

#include "world/ChatLayout.h"
#include "world/PixelBox.h"

namespace world {

// Geometría del chatbox : historial arriba y línea de tipeo
// abajo, pegadas, con las medidas del TOML.
struct ChatPanel {
	// Sin historial (o sin tipeo) la caja llega con height 0.
	PixelBox history;
	PixelBox input;

	// Renglones enteros que caben en la caja de tipeo (por si el texto
	// excede: el painter muestra los últimos).
	int inputVisibleRows = 0;
};

// Ancho de las cajas, topeado para no comerse los márgenes laterales.
int chatBoxWidth(int windowWidth, const ChatLayout& layout);

// Renglones de historial que caben arriba del input.
int maxHistoryRows(int windowHeight, int inputRows, const ChatLayout& layout);

// Cajas de la ventana: el input arranca en input_fraction y el historial
// se apila arriba suyo. inputRows = 0 es el preview (sin línea de tipeo).
ChatPanel chatPanels(int windowWidth, int windowHeight, int inputRows,
		     int historyRows, const ChatLayout& layout);

}