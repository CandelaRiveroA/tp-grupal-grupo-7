#include "JoinCodeSprites.h"

#include <QMargins>
#include <QPainter>
#include <QtGlobal>

#include "JoinCodeRects.h"
#include "NineSlice.h"

JoinCodeSprites::JoinCodeSprites() {
    QPixmap scroll(":/ui/join/scroll.png");
    if (scroll.isNull()) {
        qWarning("JoinCodeSprites: :/ui/join/scroll.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    scrollBanner_ = assembleScroll(scroll);

    QPixmap frame(":/ui/join/input_box.png");
    if (frame.isNull()) {
        qWarning("JoinCodeSprites: :/ui/join/input_box.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    inputBox_ = NineSlice(frame, QMargins(JOIN_INPUT_SLICE, JOIN_INPUT_SLICE, JOIN_INPUT_SLICE,
                                          JOIN_INPUT_SLICE))
                        .build(JOIN_INPUT_LOGICAL_W, JOIN_INPUT_LOGICAL_H, JOIN_INPUT_SCALE);

    QPixmap ok(":/ui/join/ok.png");
    if (ok.isNull()) {
        qWarning("JoinCodeSprites: :/ui/join/ok.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    okButton_ =
            ok.scaled(JOIN_OK_SIZE, JOIN_OK_SIZE, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    okButtonHover_ = brightenPercent(okButton_);
}

// Tres piezas: los rollos de los extremos se copian intactos y solo la
// columna central de 1 px se estira, para no deformar los bordes del cartel.
QPixmap JoinCodeSprites::assembleScroll(const QPixmap& scroll) const {
    const int leftW = JOIN_SCROLL_PIECE_W;
    const int rightW = JOIN_SCROLL_PIECE_W;
    const int midW = JOIN_SCROLL_LOGICAL_W - leftW - rightW;

    QPixmap banner(JOIN_SCROLL_LOGICAL_W, JOIN_SCROLL_PIECE_H);
    banner.fill(Qt::transparent);
    QPainter painter(&banner);
    painter.drawPixmap(0, 0, scroll.copy(0, 0, leftW, JOIN_SCROLL_PIECE_H));
    painter.drawPixmap(leftW, 0,
                       scroll.copy(JOIN_SCROLL_MID_X, 0, 1, JOIN_SCROLL_PIECE_H)
                               .scaled(midW, JOIN_SCROLL_PIECE_H, Qt::IgnoreAspectRatio,
                                       Qt::FastTransformation));
    painter.drawPixmap(leftW + midW, 0,
                       scroll.copy(JOIN_SCROLL_RIGHT_X, 0, rightW, JOIN_SCROLL_PIECE_H));
    painter.end();

    return banner.scaled(banner.width() * 2, banner.height() * 2, Qt::IgnoreAspectRatio,
                         Qt::FastTransformation);
}

// Nueve piezas: esquinas intactas, bordes y centro estirados

QPixmap JoinCodeSprites::brightenPercent(const QPixmap& sprite) const {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() == 0) {
                continue;
            }
            color = color.lighter(JOIN_HOVER_BRIGHT);
            image.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(image);
}

const QPixmap& JoinCodeSprites::scrollBanner() const { return scrollBanner_; }

const QPixmap& JoinCodeSprites::inputBox() const { return inputBox_; }

const QPixmap& JoinCodeSprites::okButton() const { return okButton_; }

const QPixmap& JoinCodeSprites::okButtonHover() const { return okButtonHover_; }
