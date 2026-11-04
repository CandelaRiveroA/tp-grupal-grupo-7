#include <vector>

#include "gtest/gtest.h"
#include "world/InventoryLayout.h"

namespace {

world::InventoryLayout makeBar() {
	world::InventoryLayout bar;
	bar.cellWidth = 16;
	bar.cellHeight = 16;
	bar.slotCol = 4;
	bar.slotRow = 8;
	bar.count = 9;
	bar.scale = 3;
	bar.gap = 0;
	bar.margin = 16;
	bar.anchor = world::InventoryAnchor::BottomCenter;
	return bar;
}

TEST(InventorySlotsTest, ProducesCountBoxes) {
	const world::InventoryLayout bar = makeBar();
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	ASSERT_EQ(boxes.size(), 9u);
}

TEST(InventorySlotsTest, EachBoxHasSlotDrawSize) {
	const world::InventoryLayout bar = makeBar();
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	for (const world::PixelBox& box : boxes) {
		EXPECT_EQ(box.width, 48);
		EXPECT_EQ(box.height, 48);
	}
}

TEST(InventorySlotsTest, BottomCenterHorizontalPlacement) {
	const world::InventoryLayout bar = makeBar();
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	EXPECT_EQ(boxes.front().x, 264);
	EXPECT_EQ(boxes[1].x, 264 + 48);
	EXPECT_EQ(boxes.back().x, 264 + 8 * 48);
}

TEST(InventorySlotsTest, BottomCenterVerticalPlacement) {
	const world::InventoryLayout bar = makeBar();
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	for (const world::PixelBox& box : boxes) {
		EXPECT_EQ(box.y, 576);
	}
}

TEST(InventorySlotsTest, GapSeparatesSlots) {
	world::InventoryLayout bar = makeBar();
	bar.gap = 4;
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	EXPECT_EQ(boxes[1].x - boxes[0].x, 48 + 4);
	EXPECT_EQ(bar.barWidth(), 464);
}


world::InventoryLayout makeBarWithFrame() {
	world::InventoryLayout bar;
	bar.cellWidth = 16;
	bar.cellHeight = 16;
	bar.slotCol = 4;
	bar.slotRow = 8;
	bar.count = 9;
	bar.scale = 4;
	bar.gap = 0;
	bar.margin = 16;
	bar.anchor = world::InventoryAnchor::BottomCenter;
	bar.frame.enabled = true;
	bar.frame.borderCells = 2;
	bar.frame.thickness = 16;
	bar.frame.topLeft = {1, 1};
	bar.frame.topRight = {13, 1};
	bar.frame.bottomLeft = {1, 13};
	bar.frame.bottomRight = {13, 13};
	bar.frame.top = {3, 1};
	bar.frame.bottom = {3, 13};
	bar.frame.left = {1, 3};
	bar.frame.right = {13, 3};
	return bar;
}

TEST(InventoryFrameTest, BorderThicknessAndTotalSize) {
	const world::InventoryLayout bar = makeBarWithFrame();
	EXPECT_EQ(bar.borderThickness(), 16);
	EXPECT_EQ(bar.slotsWidth(), 576);
	EXPECT_EQ(bar.totalWidth(), 608);
	EXPECT_EQ(bar.totalHeight(), 96);
}

TEST(InventoryFrameTest, OuterBoxCenteredBottom) {
	const world::InventoryLayout bar = makeBarWithFrame();
	const world::PixelBox outer = bar.outerBox(960, 640);
	EXPECT_EQ(outer.x, 176);
	EXPECT_EQ(outer.y, 528);
	EXPECT_EQ(outer.width, 608);
	EXPECT_EQ(outer.height, 96);
}

TEST(InventoryFrameTest, SlotsSitInsideTheFrame) {
	const world::InventoryLayout bar = makeBarWithFrame();
	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	ASSERT_EQ(boxes.size(), 9u);
	EXPECT_EQ(boxes.front().x, 192);
	EXPECT_EQ(boxes.front().y, 544);
	EXPECT_EQ(boxes.front().width, 64);
	EXPECT_EQ(boxes.front().height, 64);
	EXPECT_EQ(boxes.back().x, 704);
}

TEST(InventoryFrameTest, CornerAndEdgeDestinations) {
	const world::InventoryLayout bar = makeBarWithFrame();
	const world::FrameLayout fl = bar.frameLayout(960, 640);
	EXPECT_EQ(fl.corners[0].dest.x, 176);
	EXPECT_EQ(fl.corners[0].dest.y, 528);
	EXPECT_EQ(fl.corners[0].dest.width, 16);
	EXPECT_EQ(fl.corners[0].dest.height, 16);
	EXPECT_EQ(fl.corners[1].dest.x, 768);
	EXPECT_EQ(fl.edges[0].dest.x, 192);
	EXPECT_EQ(fl.edges[0].dest.width, 576);
	EXPECT_EQ(fl.edges[0].dest.height, 16);
	EXPECT_EQ(fl.edges[2].dest.height, 64);
	EXPECT_EQ(fl.corners[0].cell.col, 1);
	EXPECT_EQ(fl.edges[3].cell.col, 13);

	EXPECT_EQ(fl.corners[0].srcCols, 2);
	EXPECT_EQ(fl.corners[0].srcRows, 2);
	EXPECT_EQ(fl.edges[0].srcCols, 1);
	EXPECT_EQ(fl.edges[0].srcRows, 2);
	EXPECT_EQ(fl.edges[2].srcCols, 2);
	EXPECT_EQ(fl.edges[2].srcRows, 1);
}

TEST(InventoryFrameTest, InsetThickensFrameInward) {
	world::InventoryLayout bar = makeBarWithFrame();
	bar.frame.inset = 4;
	EXPECT_EQ(bar.slotInset(), 16);
	EXPECT_EQ(bar.totalWidth(), 608);
	EXPECT_EQ(bar.totalHeight(), 96);

	const world::PixelBox outer = bar.outerBox(960, 640);
	EXPECT_EQ(outer.x, 176);
	EXPECT_EQ(outer.y, 528);

	const std::vector<world::PixelBox> boxes = bar.slotBoxes(960, 640);
	EXPECT_EQ(boxes.front().x, 192);
	EXPECT_EQ(boxes.front().y, 544);
	EXPECT_EQ(boxes.front().width, 64);
	EXPECT_EQ(boxes.front().height, 64);

	const world::FrameLayout fl = bar.frameLayout(960, 640);
	EXPECT_EQ(fl.corners[0].dest.x, 176);
	EXPECT_EQ(fl.corners[0].dest.width, 20);
	EXPECT_EQ(fl.corners[0].dest.height, 20);
	const int frameInnerLeft = fl.corners[0].dest.x + fl.corners[0].dest.width;
	EXPECT_EQ(frameInnerLeft - boxes.front().x, 4);
}

TEST(InventoryFrameTest, UiScaleShrinksWholeBar) {
	world::InventoryLayout bar = makeBarWithFrame();
	bar.uiScale = 0.5;
	EXPECT_EQ(bar.slotDrawWidth(), 32);
	EXPECT_EQ(bar.slotDrawHeight(), 32);
	EXPECT_EQ(bar.borderThickness(), 8);
	EXPECT_EQ(bar.slotsWidth(), 288);
	EXPECT_EQ(bar.slotInset(), 8);
	EXPECT_EQ(bar.totalWidth(), 304);
}

TEST(InventoryFrameTest, UiScaleOneLeavesBaseSizes) {
	world::InventoryLayout bar = makeBarWithFrame();
	bar.uiScale = 1.0;
	EXPECT_EQ(bar.slotDrawWidth(), 64);
	EXPECT_EQ(bar.borderThickness(), 16);
}


world::InventoryLayout makeBarWithSlotFrame() {
	world::InventoryLayout bar = makeBarWithFrame();
	bar.slotFrame.enabled = true;
	bar.slotFrame.borderCells = 1;
	bar.slotFrame.thickness = 4;
	bar.slotFrame.topLeft = {8, 8};
	bar.slotFrame.topRight = {11, 8};
	bar.slotFrame.bottomLeft = {8, 11};
	bar.slotFrame.bottomRight = {11, 11};
	bar.slotFrame.top = {9, 8};
	bar.slotFrame.bottom = {9, 11};
	bar.slotFrame.left = {8, 10};
	bar.slotFrame.right = {11, 10};
	return bar;
}

TEST(InventorySlotFrameTest, PiecesSitInsideTheSlot) {
	const world::InventoryLayout bar = makeBarWithSlotFrame();
	const world::PixelBox slot{100, 100, 64, 64};
	const world::FrameLayout fl = bar.slotFrameLayout(slot);
	EXPECT_EQ(fl.corners[0].dest.x, 100);
	EXPECT_EQ(fl.corners[0].dest.y, 100);
	EXPECT_EQ(fl.corners[0].dest.width, 4);
	EXPECT_EQ(fl.corners[0].dest.height, 4);
	EXPECT_EQ(fl.corners[3].dest.x, 160);
	EXPECT_EQ(fl.corners[3].dest.y, 160);
	EXPECT_EQ(fl.edges[0].dest.x, 104);
	EXPECT_EQ(fl.edges[0].dest.width, 56);
	EXPECT_EQ(fl.edges[0].dest.height, 4);
	EXPECT_EQ(fl.corners[0].cell.col, 8);
	EXPECT_EQ(fl.edges[0].cell.col, 9);
}

TEST(InventorySlotFrameTest, UiScaleThickensInnerBorder) {
	world::InventoryLayout bar = makeBarWithSlotFrame();
	bar.uiScale = 2.0;
	const world::PixelBox slot{0, 0, 128, 128};
	const world::FrameLayout fl = bar.slotFrameLayout(slot);
	EXPECT_EQ(fl.corners[0].dest.width, 8);
	EXPECT_EQ(fl.corners[0].dest.height, 8);
}

TEST(InventorySlotFrameTest, IconBoxCenteredWithBorderPadding) {
	const world::InventoryLayout bar = makeBarWithSlotFrame();
	const world::PixelBox slot{100, 100, 64, 64};
	const world::PixelBox icon = bar.iconBox(slot);
	EXPECT_EQ(icon.x, 104);
	EXPECT_EQ(icon.y, 104);
	EXPECT_EQ(icon.width, 56);
	EXPECT_EQ(icon.height, 56);
}

TEST(InventorySlotFrameTest, IconBoxWithoutSlotFrameUsesPadding) {
	world::InventoryLayout bar = makeBarWithFrame();
	bar.slotFrame.enabled = false;
	const world::PixelBox slot{0, 0, 100, 100};
	const world::PixelBox icon = bar.iconBox(slot);
	EXPECT_EQ(icon.x, 10);
	EXPECT_EQ(icon.width, 80);
}

TEST(InventoryCountTest, EmptyWhenNoDigits) {
	const world::InventoryLayout bar = makeBar();
	const world::PixelBox slot{100, 100, 64, 64};
	EXPECT_TRUE(bar.countBoxes(slot, 0).empty());
}

TEST(InventoryCountTest, SingleDigitTopRightInsideSlot) {
	world::InventoryLayout bar = makeBar();
	bar.digitScale = 0.33;
	bar.digitMargin = 2;
	const world::PixelBox slot{100, 100, 64, 64};
	const std::vector<world::PixelBox> boxes = bar.countBoxes(slot, 1);
	ASSERT_EQ(boxes.size(), 1u);
	const int digit = 21;  // round(64*0.33)
	// Pegado arriba-derecha: x = slot derecha - margin - digit.
	EXPECT_EQ(boxes[0].width, digit);
	EXPECT_EQ(boxes[0].height, digit);
	EXPECT_EQ(boxes[0].x, 100 + 64 - 2 - digit);
	EXPECT_EQ(boxes[0].y, 102);
	// Contenido dentro del slot.
	EXPECT_GE(boxes[0].x, slot.x);
	EXPECT_LE(boxes[0].x + boxes[0].width, slot.x + slot.width);
	EXPECT_GE(boxes[0].y, slot.y);
}

TEST(InventoryCountTest, TwoDigitsAlignedRightNoOverlap) {
	world::InventoryLayout bar = makeBar();
	bar.digitScale = 0.33;
	bar.digitMargin = 2;
	const world::PixelBox slot{100, 100, 64, 64};
	const std::vector<world::PixelBox> boxes = bar.countBoxes(slot, 2);
	ASSERT_EQ(boxes.size(), 2u);
	const int digit = 21;
	// Van pegados izquierda->derecha.
	EXPECT_EQ(boxes[1].x - boxes[0].x, digit);
	// No se solapan.
	EXPECT_LE(boxes[0].x + boxes[0].width, boxes[1].x);
	// El último termina a margin del borde derecho del slot.
	EXPECT_EQ(boxes[1].x + boxes[1].width, slot.x + slot.width - 2);
	// Todo contenido dentro del slot.
	EXPECT_GE(boxes[0].x, slot.x);
	EXPECT_LE(boxes[1].x + boxes[1].width, slot.x + slot.width);
}

TEST(InventoryCountTest, ManyDigitsShrinkToFitInsideSlot) {
	world::InventoryLayout bar = makeBar();
	bar.digitScale = 0.33;
	bar.digitMargin = 2;
	const world::PixelBox slot{0, 0, 64, 64};
	// 5 dígitos a 21px no entran (105>60): deben achicarse para caber.
	const std::vector<world::PixelBox> boxes = bar.countBoxes(slot, 5);
	ASSERT_EQ(boxes.size(), 5u);
	EXPECT_LE(boxes.back().x + boxes.back().width, slot.x + slot.width);
	EXPECT_GE(boxes.front().x, slot.x);
}

}
