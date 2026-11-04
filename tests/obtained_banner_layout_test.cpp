#include "gtest/gtest.h"
#include "world/ObtainedBannerLayout.h"

namespace {

using world::BannerBox;
using world::ObtainedBannerLayout;

// Ventana 960x640, panel 64, marginX 12, stackGap 6, iconMargin 10.
ObtainedBannerLayout makeLayout() { return ObtainedBannerLayout(960, 640, 64, 12, 6, 10); }

TEST(ObtainedBannerLayoutTest, FirstPanelAnchoredLeftAndVerticallyCentered) {
    const BannerBox box = makeLayout().panelBox(0);
    EXPECT_EQ(box.x, 12);  // anclado a la izquierda
    EXPECT_EQ(box.width, 64);
    EXPECT_EQ(box.height, 64);
    EXPECT_EQ(box.y, (640 - 64) / 2);  // centrado vertical: 288
}

TEST(ObtainedBannerLayoutTest, StackGrowsDownwardByPanelPlusGap) {
    const ObtainedBannerLayout layout = makeLayout();
    const BannerBox first = layout.panelBox(0);
    const BannerBox second = layout.panelBox(1);
    const BannerBox third = layout.panelBox(2);
    EXPECT_EQ(second.x, first.x);           // misma columna
    EXPECT_EQ(second.y, first.y + 64 + 6);  // baja alto+gap
    EXPECT_EQ(third.y, first.y + 2 * (64 + 6));
}

TEST(ObtainedBannerLayoutTest, IconCenteredWithMargin) {
    const ObtainedBannerLayout layout = makeLayout();
    const BannerBox panel = layout.panelBox(0);
    const BannerBox icon = layout.iconBox(panel);
    // lado = 64 - 2*10 = 44
    EXPECT_EQ(icon.width, 44);
    EXPECT_EQ(icon.height, 44);
    // centrado: x = panel.x + (64-44)/2 = 12 + 10 = 22
    EXPECT_EQ(icon.x, panel.x + (64 - 44) / 2);
    EXPECT_EQ(icon.y, panel.y + (64 - 44) / 2);
}

TEST(ObtainedBannerLayoutTest, IconFallsBackWhenMarginTooLarge) {
    // Margen más grande que medio panel: no debe dar lado <= 0.
    ObtainedBannerLayout layout(960, 640, 20, 12, 6, 50);
    const BannerBox panel = layout.panelBox(0);
    const BannerBox icon = layout.iconBox(panel);
    EXPECT_GT(icon.width, 0);
    EXPECT_GT(icon.height, 0);
}

TEST(ObtainedBannerLayoutTest, IconOffsetShiftsIcon) {
    // Con offset (x=+3, y=-2) el icono se corre respecto del centrado.
    ObtainedBannerLayout base(960, 640, 64, 12, 6, 10);
    ObtainedBannerLayout shifted(960, 640, 64, 12, 6, 10, 3, -2);
    const BannerBox panel = base.panelBox(0);
    const BannerBox centered = base.iconBox(panel);
    const BannerBox moved = shifted.iconBox(panel);
    EXPECT_EQ(moved.x, centered.x + 3);
    EXPECT_EQ(moved.y, centered.y - 2);
    EXPECT_EQ(moved.width, centered.width);
    EXPECT_EQ(moved.height, centered.height);
}

}  // namespace
