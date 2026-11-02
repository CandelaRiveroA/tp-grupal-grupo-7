#include "CharacterSprites.h"

#include <QColor>
#include <QMargins>
#include <QtGlobal>

#include "CharacterRects.h"
#include "NineSlice.h"

CharacterSprites::CharacterSprites() {
    // La hoja trae dos fondos lado a lado (diurno | nocturno); se usa el diurno.
    QPixmap backgrounds(":/ui/character/backgrounds.png");
    if (backgrounds.isNull()) {
        qWarning("CharacterSprites: :/ui/character/backgrounds.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    portraitBackground_ = backgrounds.copy(0, 0, CHAR_PREVIEW_W, CHAR_PREVIEW_H);

    QPixmap left(":/ui/character/arrow_left.png");
    if (left.isNull()) {
        qWarning("CharacterSprites: :/ui/character/arrow_left.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    leftArrow_ =
            left.scaled(CHAR_ARROW_W, CHAR_ARROW_H, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    leftArrowHover_ = brightenPercent(leftArrow_);

    QPixmap right(":/ui/character/arrow_right.png");
    if (right.isNull()) {
        qWarning("CharacterSprites: :/ui/character/arrow_right.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    rightArrow_ =
            right.scaled(CHAR_ARROW_W, CHAR_ARROW_H, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    rightArrowHover_ = brightenPercent(rightArrow_);

    QPixmap ok(":/ui/join/ok.png");
    if (ok.isNull()) {
        qWarning("CharacterSprites: :/ui/join/ok.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    okButton_ =
            ok.scaled(CHAR_OK_SIZE, CHAR_OK_SIZE, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    okButtonHover_ = brightenPercent(okButton_);

    QPixmap textBox(":/ui/character/text_box.png");
    if (textBox.isNull()) {
        qWarning("CharacterSprites: :/ui/character/text_box.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    const QPixmap textBoxLogical = textBox.scaled(textBox.width() / CHAR_INPUT_SOURCE_DIV,
                                                  textBox.height() / CHAR_INPUT_SOURCE_DIV,
                                                  Qt::IgnoreAspectRatio, Qt::FastTransformation);
    const QMargins margins(CHAR_INPUT_MARGIN_LEFT, CHAR_INPUT_MARGIN_TOP, CHAR_INPUT_MARGIN_RIGHT,
                           CHAR_INPUT_MARGIN_BOTTOM);
    inputBox_ = NineSlice(textBoxLogical, margins)
                        .build(CHAR_INPUT_LOGICAL_W, CHAR_INPUT_LOGICAL_H, CHAR_INPUT_SCALE);

    QPixmap frame(":/ui/join/input_box.png");
    if (frame.isNull()) {
        qWarning("CharacterSprites: :/ui/join/input_box.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }
    const QMargins slice(CHAR_WINDOW_SLICE, CHAR_WINDOW_SLICE, CHAR_WINDOW_SLICE,
                         CHAR_WINDOW_SLICE);
    windowBox_ = NineSlice(frame, slice)
                         .build(CHAR_WINDOW_LOGICAL_W, CHAR_WINDOW_LOGICAL_H, CHAR_WINDOW_SCALE);
}

QPixmap CharacterSprites::brightenPercent(const QPixmap& sprite) const {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() == 0) {
                continue;
            }
            color = color.lighter(CHAR_HOVER_BRIGHT);
            image.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(image);
}

const QPixmap& CharacterSprites::portraitBackground() const { return portraitBackground_; }

const QPixmap& CharacterSprites::leftArrow() const { return leftArrow_; }

const QPixmap& CharacterSprites::leftArrowHover() const { return leftArrowHover_; }

const QPixmap& CharacterSprites::rightArrow() const { return rightArrow_; }

const QPixmap& CharacterSprites::rightArrowHover() const { return rightArrowHover_; }

const QPixmap& CharacterSprites::okButton() const { return okButton_; }

const QPixmap& CharacterSprites::okButtonHover() const { return okButtonHover_; }

const QPixmap& CharacterSprites::inputBox() const { return inputBox_; }

const QPixmap& CharacterSprites::windowBox() const { return windowBox_; }
