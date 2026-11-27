#include "CharacterSprites.h"

#include <QColor>
#include <QMargins>
#include <QtGlobal>

#include "CharacterRects.h"
#include "qt_theme/NineSlice.h"

CharacterSprites::CharacterSprites() {
    // La hoja trae dos fondos lado a lado (diurno | nocturno); se usa el diurno.
    portraitBackground_ =
            load(":/ui/character/backgrounds.png").copy(0, 0, CHAR_PREVIEW_W, CHAR_PREVIEW_H);
    leftArrow_ = load(":/ui/character/arrow_left.png")
                         .scaled(CHAR_ARROW_W, CHAR_ARROW_H, Qt::IgnoreAspectRatio,
                                 Qt::FastTransformation);
    leftArrowHover_ = brightenPercent(leftArrow_);
    rightArrow_ = load(":/ui/character/arrow_right.png")
                          .scaled(CHAR_ARROW_W, CHAR_ARROW_H, Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation);
    rightArrowHover_ = brightenPercent(rightArrow_);
    okButton_ = load(":/ui/join/ok.png")
                        .scaled(CHAR_OK_SIZE, CHAR_OK_SIZE, Qt::IgnoreAspectRatio,
                                Qt::FastTransformation);
    okButtonHover_ = brightenPercent(okButton_);
    inputBox_ = inputBoxFrom(load(":/ui/character/text_box.png"));
    windowBox_ = NineSlice(load(":/ui/join/input_box.png"),
                           QMargins(CHAR_WINDOW_SLICE, CHAR_WINDOW_SLICE, CHAR_WINDOW_SLICE,
                                    CHAR_WINDOW_SLICE))
                         .build(CHAR_WINDOW_LOGICAL_W, CHAR_WINDOW_LOGICAL_H, CHAR_WINDOW_SCALE);
}

QPixmap CharacterSprites::load(const char* path) const {
    QPixmap sprite(path);
    if (sprite.isNull()) {
        qWarning("CharacterSprites: %s no carga (alias mal escrito en el .qrc o archivo ausente).",
                 path);
    }
    return sprite;
}

QPixmap CharacterSprites::inputBoxFrom(const QPixmap& source) const {
    const QPixmap logical = source.scaled(source.width() / CHAR_INPUT_SOURCE_DIV,
                                          source.height() / CHAR_INPUT_SOURCE_DIV,
                                          Qt::IgnoreAspectRatio, Qt::FastTransformation);
    const QMargins margins(CHAR_INPUT_MARGIN_LEFT, CHAR_INPUT_MARGIN_TOP, CHAR_INPUT_MARGIN_RIGHT,
                           CHAR_INPUT_MARGIN_BOTTOM);
    return NineSlice(logical, margins)
            .build(CHAR_INPUT_LOGICAL_W, CHAR_INPUT_LOGICAL_H, CHAR_INPUT_SCALE);
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
