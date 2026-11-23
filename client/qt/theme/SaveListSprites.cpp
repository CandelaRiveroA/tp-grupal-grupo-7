#include "SaveListSprites.h"

#include <QColor>
#include <QMargins>
#include <QtGlobal>

#include "qt_theme/NineSlice.h"
#include "SaveListRects.h"

SaveListSprites::SaveListSprites():
        windowBox_(NineSlice(load(":/ui/join/input_box.png"),
                             QMargins(SAVE_WINDOW_SLICE, SAVE_WINDOW_SLICE, SAVE_WINDOW_SLICE,
                                      SAVE_WINDOW_SLICE))
                           .build(SAVE_WINDOW_LOGICAL_W, SAVE_WINDOW_LOGICAL_H, SAVE_WINDOW_SCALE)),
        rowBox_(NineSlice(load(":/ui/character/recessed_box.png"),
                          QMargins(SAVE_ROW_SLICE, SAVE_ROW_SLICE, SAVE_ROW_SLICE, SAVE_ROW_SLICE))
                        .build(SAVE_ROW_LOGICAL_W, SAVE_ROW_LOGICAL_H, SAVE_ROW_SCALE)),
        rowBoxHover_(tintRowHover(rowBox_)),
        upArrow_(load(":/ui/save/arrow_up.png")
                         .scaled(SAVE_SCROLL_ARROW_W, SAVE_SCROLL_ARROW_H, Qt::IgnoreAspectRatio,
                                 Qt::FastTransformation)),
        upArrowHover_(brightenPercent(upArrow_)),
        downArrow_(load(":/ui/save/arrow_down.png")
                           .scaled(SAVE_SCROLL_ARROW_W, SAVE_SCROLL_ARROW_H, Qt::IgnoreAspectRatio,
                                   Qt::FastTransformation)),
        downArrowHover_(brightenPercent(downArrow_)),
        deleteIcon_(load(":/ui/save/delete.png")
                            .scaled(SAVE_DELETE_SIZE, SAVE_DELETE_SIZE, Qt::IgnoreAspectRatio,
                                    Qt::FastTransformation)),
        deleteIconHover_(brightenPercent(deleteIcon_)),
        coinIcon_(load(":/ui/save/coin.png")
                          .scaled(SAVE_COIN_W, SAVE_COIN_H, Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation)),
        clockIcon_(load(":/ui/save/clock.png")
                           .scaled(SAVE_CLOCK_W, SAVE_CLOCK_H, Qt::IgnoreAspectRatio,
                                   Qt::FastTransformation)),
        coopBadge_(load(":/ui/save/coop_badge.png")
                           .scaled(SAVE_COOP_BADGE_W, SAVE_COOP_BADGE_H, Qt::IgnoreAspectRatio,
                                   Qt::FastTransformation)) {}

QPixmap SaveListSprites::load(const char* path) const {
    QPixmap sprite(path);
    if (sprite.isNull()) {
        qWarning("SaveListSprites: %s no carga (alias mal escrito en el .qrc "
                 "o archivo ausente).",
                 path);
    }
    return sprite;
}


QPixmap SaveListSprites::tintRowHover(const QPixmap& row) const {
    QImage image = row.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() == 0) {
                continue;
            }
            const int average =
                    (color.red() + color.green() + color.blue()) / SAVE_RGB_CHANNELS;
            if (average < SAVE_HOVER_KEEP_MAX) {
                continue;
            }
            color.setRed(color.red() * SAVE_HOVER_TINT_R / SAVE_PERCENT);
            color.setGreen(color.green() * SAVE_HOVER_TINT_G / SAVE_PERCENT);
            color.setBlue(color.blue() * SAVE_HOVER_TINT_B / SAVE_PERCENT);
            image.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(image);
}

QPixmap SaveListSprites::brightenPercent(const QPixmap& sprite) const {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() == 0) {
                continue;
            }
            color = color.lighter(SAVE_HOVER_BRIGHT);
            image.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(image);
}

const QPixmap& SaveListSprites::windowBox() const { return windowBox_; }

const QPixmap& SaveListSprites::rowBox() const { return rowBox_; }

const QPixmap& SaveListSprites::rowBoxHover() const { return rowBoxHover_; }

const QPixmap& SaveListSprites::upArrow() const { return upArrow_; }

const QPixmap& SaveListSprites::upArrowHover() const { return upArrowHover_; }

const QPixmap& SaveListSprites::downArrow() const { return downArrow_; }

const QPixmap& SaveListSprites::downArrowHover() const { return downArrowHover_; }

const QPixmap& SaveListSprites::deleteIcon() const { return deleteIcon_; }

const QPixmap& SaveListSprites::deleteIconHover() const { return deleteIconHover_; }

const QPixmap& SaveListSprites::coinIcon() const { return coinIcon_; }

const QPixmap& SaveListSprites::clockIcon() const { return clockIcon_; }

const QPixmap& SaveListSprites::coopBadge() const { return coopBadge_; }
