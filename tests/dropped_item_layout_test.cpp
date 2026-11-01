#include "gtest/gtest.h"

#include "world/DroppedItemLayout.h"

namespace {

using world::DroppedItemLayout;
using world::PixelRect;

TEST(DroppedItemLayoutTest, ItemCentersAndSitsOnTheCellFloor) {
    const DroppedItemLayout layout;
    // Sin drawn ni offset_y: la mitad de la celda y pegado a los pies
    // (como el huevo del piso, que no tiene aire en su sprite).
    const world::SpriteVisual visual;

    const PixelRect box = layout.rectFor(PixelRect{32, 64, 32, 32}, visual);

    EXPECT_EQ(box.width, 16);
    EXPECT_EQ(box.height, 16);
    // Centrado en la celda y apoyado abajo (como se ve en el piso).
    EXPECT_EQ(box.x, 40);
    EXPECT_EQ(box.y, 80);
}

TEST(DroppedItemLayoutTest, DrawnSizeOverridesAndOffsetShifts) {
    const DroppedItemLayout layout;
    world::SpriteVisual visual;
    visual.drawnWidth = 20;
    visual.drawnHeight = 10;
    // El ajuste vertical lo lleva el sprite (compensa su aire).
    visual.offsetY = -4;

    const PixelRect box = layout.rectFor(PixelRect{32, 64, 32, 32}, visual);

    EXPECT_EQ(box.width, 20);
    EXPECT_EQ(box.height, 10);
    EXPECT_EQ(box.x, 38);
    EXPECT_EQ(box.y, 82);
}

}  // namespace
