#include "world/InventoryLayout.h"

#include <cstddef>

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
	PixelBox box;
	box.width = totalWidth();
	box.height = totalHeight();
	box.x = (windowWidth - box.width) / 2;
	box.y = windowHeight - margin - box.height;
	return box;
}

std::vector<PixelBox> InventoryLayout::slotBoxes(int windowWidth,
						 int windowHeight) const {
	std::vector<PixelBox> boxes;
	if (count <= 0) {
		return boxes;
	}
	const PixelBox outer = outerBox(windowWidth, windowHeight);
	const int off = slotInset();
	const int w = slotDrawWidth();
	const int h = slotDrawHeight();
	const int g = gapPx();
	const int startX = outer.x + off;
	const int y = outer.y + off;
	boxes.reserve(static_cast<std::size_t>(count));
	for (int i = 0; i < count; ++i) {
		PixelBox box;
		box.x = startX + i * (w + g);
		box.y = y;
		box.width = w;
		box.height = h;
		boxes.push_back(box);
	}
	return boxes;
}

std::vector<PixelBox> InventoryLayout::slotBoxesNatural(
	int windowWidth, int windowHeight) const {
	return slotBoxes(windowWidth, windowHeight);
}

FrameLayout InventoryLayout::frameLayout(int windowWidth,
					 int windowHeight) const {
	FrameLayout fl{};
	const PixelBox outer = outerBox(windowWidth, windowHeight);
	const int t = borderThickness();
	const int n = frame.borderCells;

	const PixelBox fb{outer.x, outer.y, outer.width, outer.height};
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

}
