#pragma once

#include <QPixmap>

#include "TitleSpriteRects.h"

// Recorta el atlas de botones (:ui/title/atlas.png) y entrega los sprites
// de la pantalla de inicio ya escalados x2 con filtro nearest
class TitleSprites {
public:
    TitleSprites();

    const QPixmap& logo() const;
    const QPixmap& button(int index) const;
    const QPixmap& buttonHover(int index) const;
    const QPixmap& backButton() const;
    const QPixmap& backButtonHover() const;

private:
    QPixmap scaledTimesTwo(const QPixmap& sprite) const;
    QPixmap nullPixmap_;
    QPixmap logo_;
    QPixmap buttons_[ATLAS_BUTTON_COUNT];
    QPixmap buttonsHover_[ATLAS_BUTTON_COUNT];
    QPixmap back_;
    QPixmap backHover_;
};
