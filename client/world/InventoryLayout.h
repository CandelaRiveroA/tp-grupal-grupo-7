#pragma once

#include <array>
#include <string>
#include <vector>

#include "world/ItemCatalog.h"

namespace world {

enum class InventoryAnchor {
	BottomCenter,
};

struct PixelBox {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

struct SheetCell {
	int col = 0;
	int row = 0;
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

	FrameCells frame;

	FrameCells slotFrame;

	FrameCells activeSlotFrame;

	ItemCatalog items;


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

	std::vector<PixelBox> slotBoxesNatural(int windowWidth,
					       int windowHeight) const;

	FrameLayout frameLayout(int windowWidth, int windowHeight) const;

	FrameLayout slotFrameLayout(const PixelBox& slot) const;

	FrameLayout activeSlotFrameLayout(const PixelBox& slot) const;

	PixelBox iconBox(const PixelBox& slot) const;

private:
	FrameLayout nineSliceInside(const PixelBox& box, int t, int cells,
				    const FrameCells& f,
				    bool tileEdges) const;
};

}
