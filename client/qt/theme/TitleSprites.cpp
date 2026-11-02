#include "TitleSprites.h"

#include <QtGlobal>

TitleSprites::TitleSprites() {
    QPixmap atlas(":/ui/title/atlas.png");
    if (atlas.isNull()) {
        qWarning("TitleSprites: el atlas :/ui/title/atlas.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente).");
        return;
    }

    logo_ = scaledTimesTwo(atlas.copy(ATLAS_LOGO_X, ATLAS_LOGO_Y, ATLAS_LOGO_W, ATLAS_LOGO_H));

    for (int i = 0; i < ATLAS_BUTTON_COUNT; ++i) {
        const int x = ATLAS_BUTTON_W * i;
        buttons_[i] = scaledTimesTwo(
                atlas.copy(x, ATLAS_BUTTON_NORMAL_Y, ATLAS_BUTTON_W, ATLAS_BUTTON_H));
        buttonsHover_[i] =
                scaledTimesTwo(atlas.copy(x, ATLAS_BUTTON_HOVER_Y, ATLAS_BUTTON_W, ATLAS_BUTTON_H));
    }

    back_ = scaledTimesTwo(
            atlas.copy(ATLAS_BACK_X, ATLAS_BACK_Y_NORMAL, ATLAS_BACK_W, ATLAS_BACK_H));
    backHover_ = scaledTimesTwo(
            atlas.copy(ATLAS_BACK_X, ATLAS_BACK_Y_HOVER, ATLAS_BACK_W, ATLAS_BACK_H));
}

const QPixmap& TitleSprites::logo() const { return logo_; }

const QPixmap& TitleSprites::button(int index) const {
    if (index < 0 || index >= ATLAS_BUTTON_COUNT) {
        qWarning("TitleSprites: indice de boton fuera de rango (%d).", index);
        return nullPixmap_;
    }
    return buttons_[index];
}

const QPixmap& TitleSprites::buttonHover(int index) const {
    if (index < 0 || index >= ATLAS_BUTTON_COUNT) {
        qWarning("TitleSprites: indice de boton fuera de rango (%d).", index);
        return nullPixmap_;
    }
    return buttonsHover_[index];
}

const QPixmap& TitleSprites::backButton() const { return back_; }

const QPixmap& TitleSprites::backButtonHover() const { return backHover_; }

QPixmap TitleSprites::scaledTimesTwo(const QPixmap& sprite) const {
    return sprite.scaled(sprite.width() * 2, sprite.height() * 2, Qt::IgnoreAspectRatio,
                         Qt::FastTransformation);
}
