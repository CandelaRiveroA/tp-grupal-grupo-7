#pragma once

#include <string>

namespace world {

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
};

}
