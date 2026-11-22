#include "world/InventoryLayout.h"

#include <cstddef>

#include "world/AnchorLayout.h"
#include "world/WorldConstants.h"

namespace world {

int InventoryLayout::scaled(int basePx) const {
	return static_cast<int>(basePx * uiScale + 0.5);
}

int InventoryLayout::slotDrawWidth() const {
	return scaled(cellWidth * scale);
}

int InventoryLayout::slotDrawHeight() const {
	return scaled(cellHeight * scale);
}

int InventoryLayout::gapPx() const { return scaled(gap); }

int InventoryLayout::borderThickness() const {
	if (!frame.enabled) {
		return 0;
	}
	return scaled(frame.thickness);
}

int InventoryLayout::overlapPx() const {
	return frame.enabled ? scaled(frame.inset) : 0;
}

int InventoryLayout::slotsWidth() const {
	if (count <= 0) {
		return 0;
	}
	return count * slotDrawWidth() + (count - 1) * gapPx();
}

int InventoryLayout::slotInset() const { return borderThickness(); }

int InventoryLayout::totalWidth() const {
	return slotsWidth() + 2 * slotInset();
}

int InventoryLayout::totalHeight() const {
	return slotDrawHeight() + 2 * slotInset();
}

int InventoryLayout::barWidth() const { return slotsWidth(); }

PixelBox InventoryLayout::outerBox(int windowWidth, int windowHeight) const {
	// La barra se ancla abajo-centro (el loader del inventario solo
	// acepta "bottom_center"). La geometría la resuelve AnchorLayout:
	// la MISMA fórmula de anclaje que overlays y banners (§9, una sola
	// cuenta compartida; el render no reimplementa aritmética).
	const AnchorLayout anchors;
	return anchors.box(Anchor::BottomCenter, 0, margin, totalWidth(),
			   totalHeight(), windowWidth, windowHeight);
}

std::vector<PixelBox> InventoryLayout::slotBoxes(int windowWidth,
						 int windowHeight) const {
	// El caso de la barra: una fila de `count` slots en su caja.
	return slotBoxes(outerBox(windowWidth, windowHeight), count, 1);
}

std::vector<PixelBox> InventoryLayout::slotBoxes(const PixelBox& outer,
						 const int columns,
						 const int rows) const {
	// La grilla general: slots fila por fila, con su separación,
	// dentro del inset del marco. La barra es (count, 1); un panel 2x9
	// o 3x4 es la misma cuenta con otras dimensiones.
	std::vector<PixelBox> boxes;
	if (columns <= 0 || rows <= 0) {
		return boxes;
	}
	const int off = slotInset();
	const int w = slotDrawWidth();
	const int h = slotDrawHeight();
	const int g = gapPx();
	boxes.reserve(static_cast<std::size_t>(columns * rows));
	for (int row = 0; row < rows; ++row) {
		for (int column = 0; column < columns; ++column) {
			PixelBox box;
			box.x = outer.x + off + column * (w + g);
			box.y = outer.y + off + row * (h + g);
			box.width = w;
			box.height = h;
			boxes.push_back(box);
		}
	}
	return boxes;
}

std::vector<PixelBox> InventoryLayout::slotBoxesNatural(
	int windowWidth, int windowHeight) const {
	return slotBoxes(windowWidth, windowHeight);
}

FrameLayout InventoryLayout::frameLayout(int windowWidth,
					 int windowHeight) const {
	// El caso de la barra: el marco sobre la caja que ocupa el
	// inventario. El cálculo vive en frameLayout(box).
	return frameLayout(outerBox(windowWidth, windowHeight));
}

PixelBox InventoryLayout::centeredBox(const int windowWidth,
				      const int windowHeight,
				      const int columns, const int rows) const {
	// La caja de la grilla pedida, centrada en la ventana.
	const PixelBox grid = gridBox(columns, rows);
	PixelBox box;
	box.width = grid.width;
	box.height = grid.height;
	box.x = (windowWidth - box.width) / 2;
	box.y = (windowHeight - box.height) / 2;
	return box;
}

PixelBox InventoryLayout::gridBox(const int columns, const int rows) const {
	// Lo que ocupa una grilla columns×rows: los slots, sus
	// separaciones y el inset del marco.
	PixelBox box;
	box.width = columns * slotDrawWidth() + (columns - 1) * gapPx() +
		    2 * slotInset();
	box.height = rows * slotDrawHeight() + (rows - 1) * gapPx() +
		     2 * slotInset();
	return box;
}

int InventoryLayout::rowsFitting(const int heightPx) const {
	int rows = 1;
	while (rows < kMaxPanelRows && gridBox(1, rows).height < heightPx) {
		++rows;
	}
	return rows;
}

FrameLayout InventoryLayout::frameLayout(const PixelBox& box) const {
	FrameLayout fl{};
	const int t = borderThickness();
	const int n = frame.borderCells;

	const PixelBox fb{box.x, box.y, box.width, box.height};
	const int tb = t + overlapPx();

	fl.corners[0] = {frame.topLeft, n, n, {fb.x, fb.y, tb, tb}, false};
	fl.corners[1] = {frame.topRight, n, n,
			 {fb.x + fb.width - tb, fb.y, tb, tb}, false};
	fl.corners[2] = {frame.bottomLeft, n, n,
			 {fb.x, fb.y + fb.height - tb, tb, tb}, false};
	fl.corners[3] = {frame.bottomRight, n, n,
			 {fb.x + fb.width - tb, fb.y + fb.height - tb, tb, tb},
			 false};

	const int innerW = fb.width - 2 * tb;
	const int innerH = fb.height - 2 * tb;
	fl.edges[0] = {frame.top, 1, n, {fb.x + tb, fb.y, innerW, tb}, false};
	fl.edges[1] = {frame.bottom, 1, n,
		       {fb.x + tb, fb.y + fb.height - tb, innerW, tb}, false};
	fl.edges[2] = {frame.left, n, 1, {fb.x, fb.y + tb, tb, innerH}, false};
	fl.edges[3] = {frame.right, n, 1,
		       {fb.x + fb.width - tb, fb.y + tb, tb, innerH}, false};
	return fl;
}

FrameLayout InventoryLayout::nineSliceInside(const PixelBox& box, int t,
					     int cells, const FrameCells& f,
					     bool tileEdges) const {
	FrameLayout fl{};
	const int n = cells;
	fl.corners[0] = {f.topLeft, n, n, {box.x, box.y, t, t}, false};
	fl.corners[1] = {f.topRight, n, n,
			 {box.x + box.width - t, box.y, t, t}, false};
	fl.corners[2] = {f.bottomLeft, n, n,
			 {box.x, box.y + box.height - t, t, t}, false};
	fl.corners[3] = {f.bottomRight, n, n,
			 {box.x + box.width - t, box.y + box.height - t, t, t},
			 false};
	const int innerW = box.width - 2 * t;
	const int innerH = box.height - 2 * t;
	fl.edges[0] = {f.top, 1, n, {box.x + t, box.y, innerW, t}, tileEdges};
	fl.edges[1] = {f.bottom, 1, n,
		       {box.x + t, box.y + box.height - t, innerW, t},
		       tileEdges};
	fl.edges[2] = {f.left, n, 1, {box.x, box.y + t, t, innerH}, tileEdges};
	fl.edges[3] = {f.right, n, 1,
		       {box.x + box.width - t, box.y + t, t, innerH},
		       tileEdges};
	return fl;
}

FrameLayout InventoryLayout::slotFrameLayout(const PixelBox& slot) const {
	const int t = scaled(slotFrame.thickness);
	return nineSliceInside(slot, t, slotFrame.borderCells, slotFrame,
			       true);
}

FrameLayout InventoryLayout::activeSlotFrameLayout(const PixelBox& slot) const {
	const int t = scaled(activeSlotFrame.thickness);
	return nineSliceInside(slot, t, activeSlotFrame.borderCells,
			       activeSlotFrame, false);
}

PixelBox InventoryLayout::iconBox(const PixelBox& slot) const {
	int pad;
	if (itemPad >= 0) {
		pad = scaled(itemPad);
	} else if (slotFrame.enabled) {
		pad = scaled(slotFrame.thickness);
	} else {
		pad = slot.width / 10;
	}
	PixelBox box;
	box.x = slot.x + pad;
	box.y = slot.y + pad;
	box.width = slot.width - 2 * pad;
	box.height = slot.height - 2 * pad;
	if (box.width < 0) {
		box.width = 0;
	}
	if (box.height < 0) {
		box.height = 0;
	}
	return box;
}

std::vector<PixelBox> InventoryLayout::countBoxes(const PixelBox& slot,
						  int digitCount) const {
	std::vector<PixelBox> boxes;
	if (digitCount <= 0) {
		return boxes;
	}

	const int marginY = scaled(digitMargin);
	// marginX: usa digitMarginX si fue configurado; si no, digitMargin.
	const int marginX = scaled(digitMarginX >= 0 ? digitMarginX
						      : digitMargin);
	// gap entre dígitos (negativo = solapamiento para compensar padding
	// interno del sprite).
	const int gap = scaled(digitGap);

	// Lado nominal de un dígito como fracción del lado del slot.
	int digit = static_cast<int>(slot.width * digitScale + 0.5);
	if (digit < 1) {
		digit = 1;
	}

	// stride: avance horizontal entre el origen de un dígito y el
	// siguiente. Con gap=0 es igual al ancho del dígito (pegados).
	const int stride = digit + gap;

	// Ancho total del bloque: N dígitos + (N-1) gaps entre ellos.
	// El número se apoya abajo-derecha. Si no entra, se achica.
	const int totalWidthPx =
		(digitCount > 0) ? digit + (digitCount - 1) * stride : 0;
	const int available = slot.width - marginX;
	if (available > 0 && totalWidthPx > available && digitCount > 0) {
		// Resuelve digit tal que digit + (digitCount-1)*(digit+gap) <=
		// available  =>  digit * digitCount + (digitCount-1)*gap <=
		// available  =>  digit <= (available - (digitCount-1)*gap) /
		// digitCount
		const int num = available - (digitCount - 1) * gap;
		digit = (num > 0) ? num / digitCount : 1;
		if (digit < 1) {
			digit = 1;
		}
	}

	const int strideAdj = digit + gap;
	const int totalAdj =
		(digitCount > 0) ? digit + (digitCount - 1) * strideAdj : 0;
	const int startX = slot.x + slot.width - marginX - totalAdj;
	const int y = slot.y + slot.height - marginY - digit;

	boxes.reserve(static_cast<std::size_t>(digitCount));
	for (int i = 0; i < digitCount; ++i) {
		PixelBox box;
		box.x = startX + i * strideAdj;
		box.y = y;
		box.width = digit;
		box.height = digit;
		boxes.push_back(box);
	}
	return boxes;
}

}
