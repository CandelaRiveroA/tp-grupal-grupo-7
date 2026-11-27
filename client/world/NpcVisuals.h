#pragma once

#include <string>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/PixelBox.h"
#include "world/ScenePixelMapper.h"
#include "world/ViewDirection.h"

namespace world {

struct NpcBodyPiece {
	GridCell cell;
	PixelRect dest;
};

// Cuerpo estático del NPC: celdas de la hoja en orden de dibujo (fila por
// fila) y el tamaño lógico que ocupa en la grilla.
struct NpcBody {
	std::string sheet;
	int cellWidth = 0;
	int cellHeight = 0;
	// Celda POR DEFECTO (key `at` de npcs.toml). La posición real la manda
	// la fuente de estado (LocalWorldView::npcPlacements); ver NpcDef::cellIn.
	GridCell at{0, 0};
	GridSpan span{1, 1};
	PixelSize drawn{0, 0};
	std::vector<GridCell> cells;
	// Objetos animados: un juego de celdas por cuadro (el cuadro 0 es
	// `cells`) y los ticks que dura cada uno. Sin cuadros: cuerpo estático.
	std::vector<std::vector<GridCell>> frames;
	int frameTicks = 0;

	// Caja y piezas del cuerpo parado en la celda `where` (la resuelta
	// por NpcDef::cellIn: la de la fuente de estado o la por defecto).
	PixelRect box(int cellPixelSize, const GridCell& where) const;
	std::vector<NpcBodyPiece> pieces(int cellPixelSize,
					 const GridCell& where,
					 int frame) const;
};

// Mueble en el que el jugador se "mete" (la cama): las filas de la pieza
// (desde 0, inclusive) que se dibujan ENCIMA de quien duerme y la pose.
// La celda donde queda acostado es la celda del mueble que manda la
// fuente (la misma que usa el server).
struct PropEnter {
	int firstFrontRow;
	int lastFrontRow;
	Direction facing;
};

struct NpcPortrait {
	std::string sheet;
	int cellWidth = 0;
	int cellHeight = 0;
	GridCell at{0, 0};
	GridSpan span{1, 1};

	// Caja destino que contiene al retrato sin deformarlo.
	PixelBox fit(const PixelBox& box) const;
};

// Botón de cerrar de un popup (la X arriba a la derecha, FUERA del panel,
// pegada a su borde superior): una celda de una hoja, su tamaño en pantalla
// y su separación del panel.
// Es presentación: todo sale del TOML; quién lo muestra y qué cierra lo
// decide el dueño del popup.
struct CloseButton {
	std::string sheet;
	int cellWidth = 0;
	int cellHeight = 0;
	GridCell cell{0, 0};
	PixelSize size{0, 0};
	// x: distancia del borde derecho del panel hacia adentro (0 = alineado
	// con el borde). y: separación entre el botón y el borde superior.
	PixelPoint inset{0, 0};

	// Caja del botón: encima del panel, alineada a su derecha.
	PixelBox box(const PixelBox& panel) const;
	// ¿El punto cae sobre el botón del panel?
	bool contains(const PixelBox& panel, const PixelPoint& point) const;
};

// Rectángulo de slots del panel del popup.
struct NpcGridBox {
	GridCell at{0, 0};
	GridSpan span{1, 1};
};

struct NpcPopupStyle {
	GridSpan grid{0, 0};
	NpcGridBox portrait;
	NpcGridBox text;
	int portraitInset = 0;
	PixelPoint portraitOffset{0, 0};
	double portraitScale = 1.0;
	int textInset = 0;
	PixelPoint textOffset{0, 0};
	int textSize = 0;
	int lineGap = 0;
	Color textColor{0, 0, 0, 255};

	PixelBox portraitBox(const PixelBox& panel) const;
	PixelBox placePortrait(const PixelBox& fitted) const;
	PixelBox textBox(const PixelBox& panel) const;

private:
	PixelBox areaBox(const PixelBox& panel, const NpcGridBox& area,
			 int inset) const;
};

}
