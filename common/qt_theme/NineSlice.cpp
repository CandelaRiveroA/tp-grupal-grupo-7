#include "NineSlice.h"

#include <QPainter>
#include <QRect>

NineSlice::NineSlice(const QPixmap& frame, const QMargins& margins):
        frame_(frame), margins_(margins) {}

QPixmap NineSlice::build(int originalWidth, int originalHeight, int scale) const {
    // Cortes de cada eje: origen, fin del margen inicial, inicio del final, extremo.
    const int srcX[4] = {0, margins_.left(), frame_.width() - margins_.right(), frame_.width()};
    const int srcY[4] = {0, margins_.top(), frame_.height() - margins_.bottom(), frame_.height()};
    const int dstX[4] = {0, margins_.left(), originalWidth - margins_.right(), originalWidth};
    const int dstY[4] = {0, margins_.top(), originalHeight - margins_.bottom(), originalHeight};

    QPixmap box(originalWidth, originalHeight);
    box.fill(Qt::transparent);
    QPainter painter(&box);
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            const QRect src(QPoint(srcX[column], srcY[row]),
                            QPoint(srcX[column + 1] - 1, srcY[row + 1] - 1));
            const QRect dst(QPoint(dstX[column], dstY[row]),
                            QPoint(dstX[column + 1] - 1, dstY[row + 1] - 1));
            if (src.isEmpty() || dst.isEmpty()) {
                continue;
            }
            // Solo se estira la pieza; las esquinas conservan su tamaño.
            painter.drawPixmap(dst.topLeft(),
                               frame_.copy(src).scaled(dst.size(), Qt::IgnoreAspectRatio,
                                                       Qt::FastTransformation));
        }
    }
    painter.end();

    return box.scaled(box.width() * scale, box.height() * scale, Qt::IgnoreAspectRatio,
                      Qt::FastTransformation);
}
