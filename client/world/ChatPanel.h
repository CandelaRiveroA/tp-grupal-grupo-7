#pragma once

#include "world/ChatLayout.h"
#include "world/PixelBox.h"

#include <vector>

namespace world {

// Geometría del chatbox: historial arriba y tipeo abajo, pegados; los
// altos son px de superficies ya renderizadas.
struct ChatPanel {
	// Sin historial (o sin tipeo) la caja llega con height 0.
	PixelBox history;
	PixelBox input;
};

// Ancho de las cajas, topeado para no comerse los márgenes laterales.
int chatBoxWidth(int windowWidth, const ChatLayout& layout);

// El input arranca en input_fraction y el historial se apila arriba suyo
ChatPanel chatPanels(int windowWidth, int windowHeight,
		     int inputSurfaceHeight,
		     const std::vector<int>& historySurfaceHeights,
		     const ChatLayout& layout);

}
