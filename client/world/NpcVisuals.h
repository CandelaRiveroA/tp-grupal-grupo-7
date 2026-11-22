#pragma once

#include <string>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/PixelBox.h"
#include "world/ScenePixelMapper.h"

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

struct NpcPortrait {
	std::string sheet;
	int cellWidth = 0;
	int cellHeight = 0;
	GridCell at{0, 0};
	GridSpan span{1, 1};

	// Caja destino que contiene al retrato sin deformarlo.
	PixelBox fit(const PixelBox& box) const;
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
