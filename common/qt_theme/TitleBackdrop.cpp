#include "TitleBackdrop.h"

#include <QPainter>

#include "CloudRects.h"
#include "TitleSpriteRects.h"

TitleBackdrop::TitleBackdrop(): wrapWidth_(TITLE_SCREEN_W) {
    QPixmap sky(":/ui/title/sky.png");
    if (sky.isNull()) {
        qWarning("TitleBackdrop: :/ui/title/sky.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
    } else {
        skySource_ = sky;
        sky_ = skySource_.scaled(TITLE_SCREEN_W, TITLE_SCREEN_H, Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
    }

    const struct CloudPlacement {
        int sprite;
        int x;
        int y;
        int w;
        int h;
    } placements[CLOUD_ZONE_COUNT] = {
            {CLOUD_ZONE_1_SPRITE, CLOUD_ZONE_1_X, CLOUD_ZONE_1_Y, CLOUD_ZONE_1_W, CLOUD_ZONE_1_H},
            {CLOUD_ZONE_2_SPRITE, CLOUD_ZONE_2_X, CLOUD_ZONE_2_Y, CLOUD_ZONE_2_W, CLOUD_ZONE_2_H},
            {CLOUD_ZONE_3_SPRITE, CLOUD_ZONE_3_X, CLOUD_ZONE_3_Y, CLOUD_ZONE_3_W, CLOUD_ZONE_3_H},
            {CLOUD_ZONE_4_SPRITE, CLOUD_ZONE_4_X, CLOUD_ZONE_4_Y, CLOUD_ZONE_4_W, CLOUD_ZONE_4_H},
            {CLOUD_ZONE_5_SPRITE, CLOUD_ZONE_5_X, CLOUD_ZONE_5_Y, CLOUD_ZONE_5_W, CLOUD_ZONE_5_H},
            {CLOUD_ZONE_6_SPRITE, CLOUD_ZONE_6_X, CLOUD_ZONE_6_Y, CLOUD_ZONE_6_W, CLOUD_ZONE_6_H},
            {CLOUD_ZONE_7_SPRITE, CLOUD_ZONE_7_X, CLOUD_ZONE_7_Y, CLOUD_ZONE_7_W, CLOUD_ZONE_7_H},
            {CLOUD_ZONE_8_SPRITE, CLOUD_ZONE_8_X, CLOUD_ZONE_8_Y, CLOUD_ZONE_8_W, CLOUD_ZONE_8_H},
            {CLOUD_ZONE_9_SPRITE, CLOUD_ZONE_9_X, CLOUD_ZONE_9_Y, CLOUD_ZONE_9_W, CLOUD_ZONE_9_H},
            {CLOUD_ZONE_10_SPRITE, CLOUD_ZONE_10_X, CLOUD_ZONE_10_Y, CLOUD_ZONE_10_W,
             CLOUD_ZONE_10_H},
    };

    QPixmap sheet(":/ui/title/clouds.png");
    if (sheet.isNull()) {
        qWarning("TitleBackdrop: :/ui/title/clouds.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }

    for (int i = 0; i < CLOUD_ZONE_COUNT; ++i) {
        const struct CloudPlacement& placement = placements[i];
        const int spriteId = placement.sprite;
        const int sx = (spriteId == 1) ? CLOUD_WHITE_1_X :
                       (spriteId == 2) ? CLOUD_WHITE_2_X :
                                         CLOUD_WHITE_3_X;
        const int sy = (spriteId == 1) ? CLOUD_WHITE_1_Y :
                       (spriteId == 2) ? CLOUD_WHITE_2_Y :
                                         CLOUD_WHITE_3_Y;
        const int sw = (spriteId == 1) ? CLOUD_WHITE_1_W :
                       (spriteId == 2) ? CLOUD_WHITE_2_W :
                                         CLOUD_WHITE_3_W;
        const int sh = (spriteId == 1) ? CLOUD_WHITE_1_H :
                       (spriteId == 2) ? CLOUD_WHITE_2_H :
                                         CLOUD_WHITE_3_H;

        const float speed = (spriteId == 1) ? float(CLOUD_DRIFT_PX_PER_S_MEDIUM) :
                            (spriteId == 2) ? float(CLOUD_DRIFT_PX_PER_S_SMALL) :
                                              float(CLOUD_DRIFT_PX_PER_S_LARGE);

        clouds_[i] = sheet.copy(sx, sy, sw, sh)
                             .scaled(placement.w, placement.h, Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
        cloudX_[i] = float(placement.x);
        cloudY_[i] = placement.y;
        cloudSpeed_[i] = speed;
    }
}

void TitleBackdrop::resize(const QSize& size) {
    if (size.isEmpty() || skySource_.isNull()) {
        return;
    }
    sky_ = skySource_.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    wrapWidth_ = size.width();
}

void TitleBackdrop::step(float seconds) {
    for (int i = 0; i < CLOUD_ZONE_COUNT; ++i) {
        cloudX_[i] -= seconds * cloudSpeed_[i];
        // Wrap por nube: al salir por la izquierda reingresa completa
        // por la derecha.
        if (cloudX_[i] < -float(clouds_[i].width())) {
            cloudX_[i] = float(wrapWidth_);
        }
    }
}

void TitleBackdrop::paint(QPainter& painter) const {
    if (!sky_.isNull()) {
        painter.drawPixmap(0, 0, sky_);
    }
    for (int i = 0; i < CLOUD_ZONE_COUNT; ++i) {
        if (!clouds_[i].isNull()) {
            painter.drawPixmap(int(cloudX_[i]), cloudY_[i], clouds_[i]);
        }
    }
}
