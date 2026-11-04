#include "gtest/gtest.h"

#include "world/DialogCatalog.h"
#include "world/DialogLayout.h"

namespace {

using world::DialogData;
using world::DialogLayout;
using world::PixelRect;

TEST(DialogLayoutTest, DialogAnchorsAboveTheEntity) {
    DialogData data;
    data.animation.sprite.drawnWidth = 64;
    data.animation.sprite.drawnHeight = 64;
    data.anchorOffset = world::PixelPoint{0, -64};

    const DialogLayout layout(data);
    const PixelRect dialog =
            layout.dialogRect(PixelRect{100, 200, 32, 32});

    // Sobre la entidad: la Y baja por el offset negativo.
    EXPECT_EQ(dialog.x, 100);
    EXPECT_EQ(dialog.y, 136);
    EXPECT_EQ(dialog.width, 64);
    EXPECT_EQ(dialog.height, 64);
}

TEST(DialogLayoutTest, ContentIsCenteredInsideTheDialog) {
    DialogData data;
    data.animation.sprite.drawnWidth = 64;
    data.animation.sprite.drawnHeight = 64;
    data.content.emplace();
    data.content->drawnWidth = 32;
    data.content->drawnHeight = 40;

    const DialogLayout layout(data);
    const PixelRect dialog = layout.dialogRect(PixelRect{0, 0, 32, 32});
    const PixelRect content = layout.contentRect(dialog);

    EXPECT_EQ(content.x, 16);
    EXPECT_EQ(content.y, 12);
    EXPECT_EQ(content.width, 32);
    EXPECT_EQ(content.height, 40);
}

}  // namespace
