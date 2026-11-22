#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "world/ContentCatalog.h"
#include "world/ContentTraits.h"
#include "world/PixelBox.h"
#include "world/SpriteVisual.h"

namespace world {

enum class InventoryAnchor {
	BottomCenter,
};

struct FrameCells {
	bool enabled = false;
	int borderCells = 0;
	int thickness = 0;
	int inset = 0;
	bool cornersOnly = false;
	SheetCell topLeft;
	SheetCell topRight;
	SheetCell bottomLeft;
	SheetCell bottomRight;
	SheetCell top;
	SheetCell bottom;
	SheetCell left;
	SheetCell right;
};

struct FrameLayout {
	struct Piece {
		SheetCell cell;
		int srcCols;
		int srcRows;
		PixelBox dest;
		bool tile = false;
	};
	std::array<Piece, 4> corners;
	std::array<Piece, 4> edges;
};

struct InventoryLayout {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;

	int slotCol = 0;
	int slotRow = 0;

	int count = 0;
	int scale = 1;
	int gap = 0;
	int margin = 0;
	InventoryAnchor anchor = InventoryAnchor::BottomCenter;

	double uiScale = 1.0;

	int itemPad = -1;

	// Número de cantidad dibujado abajo-derecha del slot (count>1).
	// digitScale:   lado de cada dígito como fracción del lado del slot.
	// digitMargin:  margen interior (px, a uiScale) desde el borde del
	//               slot hasta el número. Defaults si no están en el TOML.
	// digitMarginX: margen solo para el eje X (permite negativos para
	//               mover el número hacia la derecha del borde del slot).
	//               Si no se configura, usa digitMargin como fallback.
	// digitGap:     separación extra entre dígitos consecutivos (px, a
	//               uiScale). Negativo = solapamiento para compensar el
	//               padding interno del sprite.
	double digitScale = 0.375;
	int digitMargin = 3;
	int digitMarginX = -1;  // -1 = usar digitMargin como fallback
	int digitGap = 0;

	FrameCells frame;

	FrameCells slotFrame;

	FrameCells activeSlotFrame;

	ContentCatalog content;

	// Rasgos de cada contenido (qué se puede hacer con él), aparte de
	// la apariencia (convención §9: tablas puras, sin mezclar). Lo
	// llenan los mismos lectores del TOML (campo `droppable` de cada
	// entrada de [items.*] y los sacos de crops.toml).
	ContentTraits traits;

	// Tamaño de cada ítem dibujado en la celda del inventario (clave
	// `slot_drawn` en inventory.toml, opcional). Es presentación del
	// BAR: el `drawn` del sprite es el tamaño en el mapa y puede ser
	// distinto. Sin entrada, el HUD usa el `drawn` del sprite.
	std::map<std::string, PixelSize> slotDrawn;


	int scaled(int basePx) const;

	int slotDrawWidth() const;
	int slotDrawHeight() const;

	int gapPx() const;

	int borderThickness() const;

	int overlapPx() const;

	int slotsWidth() const;

	int slotInset() const;

	int totalWidth() const;
	int totalHeight() const;

	int barWidth() const;

	PixelBox outerBox(int windowWidth, int windowHeight) const;

	std::vector<PixelBox> slotBoxes(int windowWidth,
					int windowHeight) const;

	// Los slots en grilla columns×rows dentro de cualquier caja (el
	// caso de la barra es una fila de `count`): un panel con la figura
	// del inventario los pide acá.
	std::vector<PixelBox> slotBoxes(const PixelBox& outer, int columns,
					int rows) const;

	// La caja que ocupa una grilla columns×rows de slots (slots +
	// separaciones + el inset del marco).
	PixelBox gridBox(int columns, int rows) const;

	// Cantidad mínima de filas (al menos 1) cuya caja gridBox alcanza a
	// contener `heightPx` de contenido. Usa gridBox, no repite su fórmula.
	int rowsFitting(int heightPx) const;

	std::vector<PixelBox> slotBoxesNatural(int windowWidth,
					       int windowHeight) const;

	FrameLayout frameLayout(int windowWidth, int windowHeight) const;

	// El MISMO marco del inventario dibujado dentro de cualquier caja
	// (el caso de la barra es la caja de outerBox). Genérico: cualquier
	// panel que quiera la figura del inventario la pide acá.
	FrameLayout frameLayout(const PixelBox& box) const;

	// La caja de una grilla columns×rows de slots centrada en la
	// ventana (el popup). La barra es otro caso: outerBox (una fila de
	// `count` anclada abajo).
	PixelBox centeredBox(int windowWidth, int windowHeight, int columns,
			     int rows) const;

	FrameLayout slotFrameLayout(const PixelBox& slot) const;

	FrameLayout activeSlotFrameLayout(const PixelBox& slot) const;

	PixelBox iconBox(const PixelBox& slot) const;

	// Rects de cada dígito del número de cantidad, alineados ABAJO y a
	// la DERECHA dentro del slot, en orden izquierda->derecha. Los
	// dígitos van pegados; el ancho total se limita al slot para que un
	// número de 2+ dígitos no desborde. digitCount<=0 devuelve vacío.
	std::vector<PixelBox> countBoxes(const PixelBox& slot,
					 int digitCount) const;

private:
	FrameLayout nineSliceInside(const PixelBox& box, int t, int cells,
				    const FrameCells& f,
				    bool tileEdges) const;
};

}
